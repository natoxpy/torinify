#include "./db.c"
#include "./sh.c"
#include "./utils/generic_vec.h"
#include <stdio.h>
#include <taglib/tag_c.h>
#include <unistd.h>

int main() {
    printf("====== [ TESTS ] ======\n");

    // Vec *data = vec_init(sizeof(int));

    // int v1 = 1;
    // vec_push(data, (void *)(uintptr_t)&v1);

    // int v2 = 2;
    // vec_push(data, (void *)(uintptr_t)&v2);

    // int v3 = 3;
    // vec_push(data, (void *)(uintptr_t)&v3);

    // for (int i = 0; i < data->length; i++) {
    //     int v = *(int *)vec_get(data, i);
    //     printf("index %d value %d\n", i, v);
    // }

    // vec_free(data);

    // db_tests();
    // sh_tests();

    return 0;
    // TagLib_File *file = taglib_file_new(
    //     "/home/toxpy/Music/server/Mili/Grown-up's Paradise (2024)/Mili - "
    //     "Grown-up's Paradise - 01 - Grown-Up's Paradise.mp3");

    // TagLib_Complex_Property_Attribute ***properties =
    //     taglib_complex_property_get(file, "PICTURE");

    // TagLib_Complex_Property_Attribute ***propPtr = properties;
    // while (*propPtr) {
    //     TagLib_Complex_Property_Attribute **attrPtr = *propPtr;
    //     while (*attrPtr) {
    //         TagLib_Complex_Property_Attribute *attr = *attrPtr;
    //         switch (attr->value.type) {
    //         case TagLib_Variant_String:
    //             if (strcmp("mimeType", attr->key) == 0) {
    //                 printf("mimeType \"%s\"\n",
    //                 attr->value.value.stringValue);
    //             } else if (strcmp("description", attr->key) == 0) {
    //                 printf("Description \"%s\"\n",
    //                        attr->value.value.stringValue);
    //             } else if (strcmp("pictureType", attr->key) == 0) {
    //                 printf("pictureType \"%s\"\n",
    //                        attr->value.value.stringValue);
    //             }
    //             break;
    //         case TagLib_Variant_ByteVector:
    //             if (strcmp("data", attr->key) == 0) {
    //                 // picture->data = attr->value.value.byteVectorValue;
    //                 // picture->size = attr->value.size;
    //             }
    //             break;
    //         default:
    //             break;
    //         }
    //         ++attrPtr;
    //     }
    //     ++propPtr;
    // }

    // taglib_complex_property_free(properties);

    // taglib_file_free(file);

    return 0;
}
