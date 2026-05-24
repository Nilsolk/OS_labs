#include <stdio.h>
#include <stdlib.h>

#define PAGE_TABLE_SIZE 256
#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define FRAME_COUNT 256
#define TLB_SIZE 16

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s addresses.txt\n", argv[0]);
        return 1;
    }

    FILE *addresses = fopen(argv[1], "r");
    if (addresses == NULL)
    {
        perror("addresses.txt");
        return 1;
    }

    FILE *backing = fopen("BACKING_STORE.bin", "rb");
    if (backing == NULL)
    {
        perror("BACKING_STORE.bin");
        fclose(addresses);
        return 1;
    }

    int page_table[PAGE_TABLE_SIZE];
    for (int i = 0; i < PAGE_TABLE_SIZE; i++)
    {
        page_table[i] = -1;
    }

    int tlb_pages[TLB_SIZE];
    int tlb_frames[TLB_SIZE];
    for (int i = 0; i < TLB_SIZE; i++)
    {
        tlb_pages[i] = -1;
        tlb_frames[i] = -1;
    }

    signed char physical_memory[FRAME_COUNT][FRAME_SIZE];

    int next_free_frame = 0;
    int next_tlb_entry = 0;

    int total_addresses = 0;
    int page_faults = 0;
    int tlb_hits = 0;

    int logical_address;

    while (fscanf(addresses, "%d", &logical_address) == 1)
    {
        total_addresses++;

        int address_16 = logical_address & 0xFFFF;
        int page_number = (address_16 >> 8) & 0xFF;
        int offset = address_16 & 0xFF;

        int frame_number = -1;

        for (int i = 0; i < TLB_SIZE; i++)
        {
            if (tlb_pages[i] == page_number)
            {
                frame_number = tlb_frames[i];
                tlb_hits++;
                break;
            }
        }

        if (frame_number == -1)
        {
            if (page_table[page_number] == -1)
            {
                page_faults++;

                frame_number = next_free_frame;
                next_free_frame++;

                if (fseek(backing, page_number * PAGE_SIZE, SEEK_SET) != 0)
                {
                    perror("fseek");
                    return 1;
                }

                size_t bytes_read = fread(physical_memory[frame_number], sizeof(signed char), PAGE_SIZE, backing);
                if (bytes_read != PAGE_SIZE)
                {
                    printf("Error: could not read page %d\n", page_number);
                    return 1;
                }

                page_table[page_number] = frame_number;
            }
            else
            {
                frame_number = page_table[page_number];
            }

            tlb_pages[next_tlb_entry] = page_number;
            tlb_frames[next_tlb_entry] = frame_number;
            next_tlb_entry = (next_tlb_entry + 1) % TLB_SIZE;
        }

        int physical_address = frame_number * FRAME_SIZE + offset;
        signed char value = physical_memory[frame_number][offset];

        printf("Virtual address: %d Physical address: %d Value: %d\n",
               logical_address, physical_address, value);
    }

    printf("Number of Translated Addresses = %d\n", total_addresses);
    printf("Page Faults = %d\n", page_faults);
    printf("Page Fault Rate = %.3f\n", (double)page_faults / total_addresses);
    printf("TLB Hits = %d\n", tlb_hits);
    printf("TLB Hit Rate = %.3f\n", (double)tlb_hits / total_addresses);

    fclose(addresses);
    fclose(backing);

    return 0;
}
