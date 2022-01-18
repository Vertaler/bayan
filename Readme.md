Bayan is tool for searching file duplicates based on comparison of file block hashes.
Tool implemented in way to minimize amount of file data reads. Data is read only if it is actually necessary.

Command line reference:
```                                                 
  -h [ --help ]               Show help
  --dirs arg                  Directories for scan
  --exclude_dirs arg          Directories to exclude from scan
  --min_file_size arg (=1)    Minimal size of file to check for duplicates (in bytes)
  --filename_pattern arg      Name pattern for files to check for duplicates
                              (case insensitive)
  --recursion_depth arg       Maximal depth of nested directories to scan (0 means current directory only). By default unlimited.
  --hash arg (=crc32)         Hashing algoritm used for file block comparison.  Allowed values: crc16,crc32 and crc64
  --block_size arg (=4096)    Size of file block read from disk at once (in bytes)
  --log_file arg (=bayan.log) File to log some auxiliary information during utility execution
```