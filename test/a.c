#include <stdio.h>
#include <stdint.h>

static const uint64_t C[] = {
    1283868770400643928u,  6416574995475331444u,  4078260278032692663u,
    2353523259288686585u,  1227179971273316331u,   575931623374121527u,
    242543240509105209u,    91437049221049666u,    30799446349977173u,
    9255276791179340u,     2478152334826140u,      590642893610164u,
    125206034929641u,       23590435911403u,        3948334035941u,
    586753615614u,          77391054539u,           9056793210u,
    940121950u,             86539696u,              7062824u,
    510971u,                32764u,                 1862u,
    94u,                    4u,                    0u
};
int main()
{
    uint32_t l,h;
    uint32_t N = sizeof(C)/sizeof(C[0]);
    FILE* fp = fopen("uint64_s.txt","w");
    if (fp == NULL)
    {
        printf("cannot open file!\n");
        return 0;
    }
    fprintf(fp,"static const uint64_s C[] = {\n");
    for (int i = 0; i < N; i++)
    {
        l = (uint32_t)C[i];
        h = (uint32_t)(C[i] >> 32);
        if (i < N-1)
        {
            if (i%3 == 0)
                fprintf(fp,"\t{{0x%08x, 0x%08x}},",l,h);
            else if (i%3 == 1)
                fprintf(fp," {{0x%08x, 0x%08x}},",l,h);
            else if (i%3 == 2)
                fprintf(fp," {{0x%08x, 0x%08x}},\n",l,h);
        }
        else
        {
            fprintf(fp,"\t{{0x%08x, 0x%08x}}\n",l,h);
            fprintf(fp,"};\n");
        }
    }
}