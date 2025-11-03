#include <sys/mman.h>
#include <string.h>

// string concatenation function
void* __whacky_strcat(const char* left_ptr, unsigned long left_len, const char* right_ptr, unsigned long right_len, unsigned long* out_len) {
    // calculate total length
    unsigned long total_len = left_len + right_len;
    
    // allocate memory
    void* result = mmap(NULL, total_len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        return NULL;
    }
    
    // copy both strings
    memcpy(result, left_ptr, left_len);
    memcpy((char*)result + left_len, right_ptr, right_len);
    
    *out_len = total_len;
    
    return result;
}

// string multiplication function
void* __whacky_strmul(const char* str_ptr, unsigned long str_len, unsigned long n, unsigned long* out_len) {
    // calculate total length
    unsigned long total_len = str_len * n;
    
    // allocate memory
    void* result = mmap(NULL, total_len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (result == MAP_FAILED) {
        return NULL;
    }
    
    // copy the string n times
    for (unsigned long i = 0; i < n; i++) {
        memcpy((char*)result + (i * str_len), str_ptr, str_len);
    }
    
    *out_len = total_len;
    
    return result;
}
