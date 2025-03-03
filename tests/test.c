#include "./db.c"
#include "./sh.c"
#include <stdio.h>

int main() {
    printf("====== [ TESTS ] ======\n");
    db_tests();
    sh_tests();

    return 0;
}
