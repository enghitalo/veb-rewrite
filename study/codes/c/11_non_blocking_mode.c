/*
wrk -t16 -c512 -d10s http://127.0.0.1:8081

Running 10s test @ http://127.0.0.1:8081
  16 threads and 512 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     8.58ms   13.28ms 147.27ms   88.40%
    Req/Sec     1.35k     2.01k    9.69k    85.17%
  201931 requests in 10.12s, 22.72MB read
  Socket errors: connect 0, read 1137, write 0, timeout 0
Requests/sec:  19961.76
Transfer/sec:      2.25MB
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdatomic.h>

#define PORT 8081
#define BUFFER_SIZE 140
#define RESPONSE_BODY "{\"message\": \"Hello, world!\"}"
#define RESPONSE_BODY_LENGTH (sizeof(RESPONSE_BODY) - 1)
#define MAX_CONNECTION_SIZE 512
#define INITIAL_THREAD_POOL_SIZE 8
#define MAX_THREAD_POOL_SIZE 16

typedef struct HazardPointer
{
    atomic_intptr_t ptr;
} HazardPointer;

#define MAX_HAZARD_POINTERS (MAX_THREAD_POOL_SIZE * 2)
HazardPointer hazard_pointers[MAX_HAZARD_POINTERS];
pthread_mutex_t hp_lock = PTHREAD_MUTEX_INITIALIZER;

void set_hazard_pointer(int idx, int fd)
{
    atomic_store(&hazard_pointers[idx].ptr, (intptr_t)fd);
}

void clear_hazard_pointer(int idx)
{
    atomic_store(&hazard_pointers[idx].ptr, 0);
}

int is_safe_to_close(int fd)
{
    for (int i = 0; i < MAX_HAZARD_POINTERS; i++)
    {
        if (atomic_load(&hazard_pointers[i].ptr) == fd)
        {
            return 0;
        }
    }
    return 1;
}

void safe_close(int fd)
{
    pthread_mutex_lock(&hp_lock);
    if (is_safe_to_close(fd))
    {
        close(fd);
    }
    pthread_mutex_unlock(&hp_lock);
}

typedef struct ThreadArg
{
    int epoll_fd;
    int thread_id;
} ThreadArg;

// Worker thread function
void *worker_thread(void *arg)
{
    ThreadArg *targ = (ThreadArg *)arg;
    int epoll_fd = targ->epoll_fd;
    int thread_id = targ->thread_id;

    struct epoll_event events[MAX_CONNECTION_SIZE];

    while (1)
    {
        int num_events = epoll_wait(epoll_fd, events, MAX_CONNECTION_SIZE, -1);

        for (int i = 0; i < num_events; i++)
        {
            int client_fd = events[i].data.fd;
            if (events[i].events & (EPOLLHUP | EPOLLERR))
            {
                set_hazard_pointer(thread_id, client_fd);
                safe_close(client_fd);
                clear_hazard_pointer(thread_id);
                continue;
            }

            if (events[i].events & EPOLLIN)
            {
                set_hazard_pointer(thread_id, client_fd);
                char buffer[BUFFER_SIZE] = {0};
                int bytes_read;

                while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
                {
                    buffer[bytes_read] = '\0';
                    char response[BUFFER_SIZE];
                    int response_length = snprintf(response, sizeof(response),
                                                   "HTTP/1.1 200 OK\r\n"
                                                   "Content-Type: application/json\r\n"
                                                   "Content-Length: %zu\r\n"
                                                   "Connection: close\r\n\r\n"
                                                   "%s",
                                                   RESPONSE_BODY_LENGTH, RESPONSE_BODY);
                    send(client_fd, response, response_length, 0);
                }

                if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    clear_hazard_pointer(thread_id);
                    continue;
                }
                else if (bytes_read == 0)
                {
                    safe_close(client_fd);
                    clear_hazard_pointer(thread_id);
                    continue;
                }

                clear_hazard_pointer(thread_id);
            }
        }
    }
    return NULL;
}

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, MAX_CONNECTION_SIZE) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
    {
        perror("epoll_create1 failed");
        close(server_fd);
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    int num_threads = INITIAL_THREAD_POOL_SIZE;
    pthread_t threads[MAX_THREAD_POOL_SIZE];
    ThreadArg thread_args[MAX_THREAD_POOL_SIZE];
    for (int i = 0; i < num_threads; i++)
    {
        thread_args[i].epoll_fd = epoll_fd;
        thread_args[i].thread_id = i;
        pthread_create(&threads[i], NULL, worker_thread, &thread_args[i]);
    }

    while (1)
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd >= 0)
        {
            fcntl(client_fd, F_SETFL, O_NONBLOCK);
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = client_fd;
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
        }
        else if (errno != EWOULDBLOCK)
        {
            perror("Accept failed");
            break;
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_cancel(threads[i]);
    }
    close(server_fd);
    return 0;
}
