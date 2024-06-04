/*
 * Author       : weijundong
 * LastEditors  : weijundong
 * Date         : 2021-12-18 09:15:25
 * LastEditTime : 2021-12-18 14:52:49
 * FilePath     : /AWA5920c:/Users/shino/Desktop/ART-Pi/demo/main.c
 * Description  : 
 */
#include <stdio.h>
#include <stdlib.h>

typedef int (*init_fn_t)(void);

struct rt_init_desc
{
    const char* level;
    const init_fn_t fn;
};

#define RT_USED                     __attribute__((used))
#define RT_SECTION(x)               __attribute__((section(x)))

#define INIT_EXPORT(fn, level)                                                       \
            RT_USED const init_fn_t __rt_init_##fn RT_SECTION(".rti_fn." level) = fn

#define INIT_COMPONENT_EXPORT(fn)       INIT_EXPORT(fn, "1")

static int rti_board_start(void)
{
    printf("rti_board_start\n");
    return 0;
}
INIT_EXPORT(rti_board_start, "0.end");


int mycomponets(void)
{
    printf("Before the main: %s\n", __FILE__);

    return 0;
}
INIT_COMPONENT_EXPORT(mycomponets);


int yangman_good_forme(void)
{
    printf("you are good yong man\n");

    return 0;
}
INIT_COMPONENT_EXPORT(yangman_good_forme);


static int rti_board_end(void)
{
    printf("rti_board_end\n");
    return 0;
}
INIT_EXPORT(rti_board_end, "1.end");


// RT_USED const init_fn_t __rt_init_rti_board_end RT_SECTION(".rti_fn." "1.end") = rti_board_end;


int a __attribute__((section("list"))) = 125;

int main(int argc, char const *argv[])
{
    volatile const init_fn_t *fn_ptr;
    
    for (fn_ptr = &__rt_init_rti_board_start; fn_ptr <= &__rt_init_rti_board_end; fn_ptr++)
    {
        (*fn_ptr)();
    }

    printf("a = %d\n", a);

    return 0;
}
