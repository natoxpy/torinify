#include "./db.c"
#include "./sh.c"

int main() {
    printf("====== [ TESTS ] ======\n");
    db_tests();
    sh_tests();

    return 0;
}