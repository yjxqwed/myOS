#include "ata.h"
#include "simplefs.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("USAGE: fstool <img> [load|dump|rm|stat|list] ...\n");
        return -1;
    }
    disk_t *disk = disk_open(argv[1]);
    print_disk(disk);
    simplefs_init();

    if (argc >= 3) {
        char *cmd1 = argv[2];
        if (strcmp(cmd1, "load") == 0 && argc >= 5) {
            char *host_file = argv[3];
            char *guest_file = argv[4];
            FILE *f = fopen(host_file, "rb");
            if (f) {
                int fd = simplefs_file_open(guest_file, O_CREAT);
                if (fd >= 0) {
                    fseek(f, 0, SEEK_END);
                    long fsize = ftell(f);
                    fseek(f, 0, SEEK_SET);
                    void *buffer = malloc(fsize);
                    fread(buffer, fsize, 1, f);
                    int total_bytes_written = simplefs_file_write(fd, buffer, fsize);
                    printf("total_bytes_written = %d\n", total_bytes_written);
                    simplefs_file_close(fd);
                    free(buffer);
                } else {
                    printf("simplefs: failed to open %s, fd=%d\n", guest_file, fd);
                }
                fclose(f);
            } else {
                perror("No such file.");
            }
        } else if (strcmp(cmd1, "dump") == 0 && argc >= 5) {
            char *guest_file = argv[3];
            char *host_file = argv[4];
            FILE *f = fopen(host_file, "wb+");
            if (f) {
                int fd = simplefs_file_open(guest_file, 0);
                printf("simplefs: open %s, fd=%d\n", guest_file, fd);
                if (fd >= 0) {
                    int fsize = simplefs_file_lseek(fd, 0, SEEK_END);
                    simplefs_file_lseek(fd, 0, SEEK_SET);
                    void *buffer = malloc(fsize);
                    int total_bytes_read = simplefs_file_read(fd, buffer, fsize);
                    printf("total_bytes_read = %d\n", total_bytes_read);
                    simplefs_file_close(fd);
                    fwrite(buffer, fsize, 1, f);
                    free(buffer);
                } else {
                    printf("simplefs: failed to open %s, fd=%d\n", guest_file, fd);
                }
                fclose(f);
            } else {
                perror("Failed to open host file");
            }
        } else if (strcmp(cmd1, "rm") == 0 && argc >= 4) {
            char *guest_file = argv[3];
            int ok = simplefs_file_delete(guest_file);
            printf("rm %s: %d\n", guest_file, ok);
        } else if (strcmp(cmd1, "stat") == 0 && argc >= 4) {
            char *guest_file = argv[3];
            stat_t sb;
            int ok = simplefs_file_stat(guest_file, &sb);
            if (ok == 0) {
                printf("%d, %d, %d, %s\n", sb.file_id, sb.size, sb.blocks, sb.filename);
            } else {
                printf("stat %s failed: %d\n", guest_file, ok);
            }
        } else if (strcmp(cmd1, "list") == 0) {
            stat_t *buffer = malloc(4096 * sizeof(stat_t));
            int nfiles = simplefs_list_files(buffer);
            printf("simplefs_list_files: %d\n", nfiles);
            for (int i = 0; i < nfiles; i++) {
                stat_t *sb = buffer + i;
                printf("%d, %d, %d, %s\n", sb->file_id, sb->size, sb->blocks, sb->filename);
            }
            free(buffer);
        } else {
            printf("bad args\n");
        }
    } else {
        printf("bad args\n");
    }

    simplefs_close();
    disk_close(disk);
    return 0;
}