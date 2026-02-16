#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h> // getgimeofday(), for current_millis()

long long current_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

void vec_add(const size_t n,
             float out[restrict n],
             const float in1[restrict n],
             const float in2[restrict n]) {
    for(size_t i = 0; i<n; ++i) {
        out[i] = in1[i] + in2[i];
    }
}

int main() {
    // test data
    const size_t vectors_length = 200000;
    const size_t vectors_size = vectors_length * sizeof(float);
    float* host_in1 = (float*)malloc(vectors_size);
    float* host_in2 = (float*)malloc(vectors_size);
    for(size_t i = 0; i<vectors_length; ++i) {
        host_in1[i] = (float)i;
        host_in2[i] = 10.0f;
    }

    // buffer for the results
    float* host_out = (float*)malloc(vectors_size);

    puts("starting benchmark");
    long long ms = current_millis();
    for(size_t i = 0; i<4000; ++i) {
        printf("iteration [%02zu/4000]\r", i);
        vec_add(vectors_length, host_out, host_in1, host_in2);
    }
    ms = current_millis()-ms;
    printf("\ntook %lld.%lld seconds\n", (ms/1000), (ms%1000));

    free(host_in1);
    free(host_in2);
    free(host_out);

    return 0;
}
