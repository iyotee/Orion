// Utilitaire hexdump simple pour Orion
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

static void print_hex_line(const uint8_t* data, size_t offset, size_t len) {
    printf("%08zx  ", offset);
    
    // Hex bytes
    for (size_t i = 0; i < 16; i++) {
        if (i < len) {
            printf("%02x ", data[i]);
        } else {
            printf("   ");
        }
        if (i == 7) printf(" ");
    }
    
    printf(" |");
    
    // ASCII representation
    for (size_t i = 0; i < len; i++) {
        char c = data[i];
        printf("%c", (c >= 32 && c <= 126) ? c : '.');
    }
    
    printf("|\n");
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }
    
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    uint8_t buffer[16];
    size_t offset = 0;
    ssize_t bytes_read;
    
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        print_hex_line(buffer, offset, bytes_read);
        offset += bytes_read;
    }
    
    close(fd);
    return 0;
}
