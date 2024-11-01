## [open.2](https://man7.org/linux/man-pages/man2/open.2.html)
   
   Open file descriptions
       The term open file description is the one used by POSIX to refer
       to the entries in the system-wide table of open files.  In other
       contexts, this object is variously also called an "open file
       object", a "file handle", an "open file table entry", or—in
       kernel-developer parlance—a struct file.

       When a file descriptor is duplicated (using dup(2) or similar),
       the duplicate refers to the same open file description as the
       original file descriptor, and the two file descriptors
       consequently share the file offset and file status flags.  Such
       sharing can also occur between processes: a child process created
       via fork(2) inherits duplicates of its parent's file descriptors,
       and those duplicates refer to the same open file descriptions.

       Each open() of a file creates a new open file description; thus,
       there may be multiple open file descriptions corresponding to a
       file inode.

       On Linux, one can use the kcmp(2) KCMP_FILE operation to test
       whether two file descriptors (in the same process or in two
       different processes) refer to the same open file description.

## [fcntl.2](https://man7.org/linux/man-pages/man2/fcntl.2.html)

   File status flags
       Each open file description has certain associated status flags,
       initialized by open(2) and possibly modified by fcntl().
       Duplicated file descriptors (made with dup(2), fcntl(F_DUPFD),
       fork(2), etc.) refer to the same open file description, and thus
       share the same file status flags.

       The file status flags and their semantics are described in
       open(2).

       F_GETFL (void)
              Return (as the function result) the file access mode and
              the file status flags; arg is ignored.

       F_SETFL (int)
              Set the file status flags to the value specified by arg.
              File access mode (O_RDONLY, O_WRONLY, O_RDWR) and file
              creation flags (i.e., O_CREAT, O_EXCL, O_NOCTTY, O_TRUNC)
              in arg are ignored.  On Linux, this operation can change
              only the O_APPEND, O_ASYNC, O_DIRECT, O_NOATIME, and
              O_NONBLOCK flags.  It is not possible to change the
              O_DSYNC and O_SYNC flags; see BUGS, below.