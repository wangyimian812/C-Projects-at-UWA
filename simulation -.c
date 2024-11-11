//  CITS2002 Project 2 2024
//  Student1:   23845246   Yimian Wang
//  Platform:   Linux
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

//  'Memory' is the user-created data type, it has 3 fields: process_id, page_number and last_accessed
typedef struct {
    int process_id;
    int page_number;
    int last_accessed;
} Memory;

//  The size of virtual memory is 32 and the size of RAM is 16
Memory *virtual_memory[32];
Memory *RAM[16];

//  Initialise virtual memory 
void initialise_virtual_memory() {
    int position = 0; // Indicates the position so that the program would know where to place pages in virtual_memory
//  4 processes and 4 page numbers for each process
    for (int process_id = 0; process_id < 4; process_id++) {
        for (int page_number = 0; page_number < 4; page_number++) {
            Memory *page = malloc(sizeof(Memory));
            if (page == NULL) { // Error handling if memory allocation fails
                fprintf(stderr, "Error! Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            page->process_id = process_id;
            page->page_number = page_number;
            page->last_accessed = 0; // Initialise last_accessed to 0
//  Store the page in two consecutive slots in virtual_memory  
            virtual_memory[position++] = page;
            virtual_memory[position++] = page;
        }
    }
}

//  Initialise RAM
void initialise_RAM() {
    for (int i = 0; i < 16; i++) {
        RAM[i] = NULL; // Set RAM to NULL as the value will be set later 
    }
}

//  Initialise page tables
int page_table[4][4]; // 4 processes and 4 page numbers for each process
void initialise_page_table() {
    for (int process_id = 0; process_id < 4; process_id++) {
        for (int page_number = 0; page_number < 4; page_number++) {
            page_table[process_id][page_number] = 99;  // The page is in disc
        }
    }
}

//  Evict pages using the LRU algorithm
int evict_a_page(int pid, int current_access_time) {
    int oldest_access_time = current_access_time;
    int frame_to_be_evicted = 0; // Initialise the frame to be evicted to 0, which is an assumption that the 0th frame will be evicted
    bool frame_found = false;

//  There are only 8 frames in the memory 
    for (int frame = 0; frame < 8; frame++) {  
        if (RAM[frame * 2] != NULL && RAM[frame * 2]->process_id == pid) { // If the page belongs to the same pid
//  If the frame is not found or the two consecutive arrays were accessed earlier, then update oldest_access_time
            if(!frame_found || RAM[frame * 2]->last_accessed < oldest_access_time) {
               oldest_access_time = RAM[frame * 2]->last_accessed;
//  If this is the first valid frame found, or it was accessed earlier than the current oldest, update the LRU frame
               frame_to_be_evicted = frame;
               frame_found = true;
            }  
        }
    }

//  If no local least recently used page is found, check for the global least recently used page
    if (!frame_found) {
        for (int i = 0; i < 8; i++) {
            if (RAM[i * 2] != NULL) {
// Check if the current page was accessed earlier than the oldest_access_time
                if (RAM[i * 2]->last_accessed < oldest_access_time) {
// Update oldest_access_time to the current page's last accessed time
                    oldest_access_time = RAM[i * 2]->last_accessed;
// This is the frame that will be evicted 
                    frame_to_be_evicted = i;
                } 
            }
        }
    }

// Retrieve the pid of the page being evicted
    int evicted_pid = RAM[frame_to_be_evicted * 2]->process_id;
// Retrieve the page number of the page being evicted 
    int evicted_page_number = RAM[frame_to_be_evicted * 2]->page_number;
    page_table[evicted_pid][evicted_page_number] = 99; // The page is in disc
    RAM[frame_to_be_evicted * 2] = NULL; // Clear the first slot of the frame
    RAM[frame_to_be_evicted * 2 + 1] = NULL; // Clear the second slot of the frame

    return frame_to_be_evicted; // Return the frame number, the frame is emptied now free for use

}

//  Bring pages from virtual memory into RAM 
void bring_pages_from_virtual_memory_into_RAM (int pid, int page_number, int frame, int time) {
//  Calculate the page position in virtual memory, each process has 8 slots -- 4 pages, each page has 2 slots
    int virtual_memory_position = (pid * 8) + (page_number * 2);
//  Load the page into RAM
    RAM[frame * 2] = virtual_memory[virtual_memory_position];
    RAM [frame * 2 + 1] = virtual_memory[virtual_memory_position + 1];
//  Update the page table
    page_table[pid][page_number] = frame;
//  Update last_accessed time
    RAM[frame * 2]->last_accessed = time;
} 

//  Find a free frame from the RAM, or, if the RAM is full, evict a page
int find_free_frame_otherwise_evict( int pid, int time) {
//  This loop checks for a free frame in the RAM
    for (int frame = 0; frame < 8; frame++) {
//  If the frame is empty
        if (RAM[frame * 2] == NULL) {
//  Return the frame number
            return frame;
        }
    }

//  Or else (no free frame), evict a page using the LRU algorithm
    return evict_a_page(pid, time);
}

int main(int argc, char *argv[]) {

    int process_id_storage[200]; // Store process id
    int count = 0; // Count the number of process id that has been read
    int time = 1; // Initialise time to 1 to avoid the last access time starting from 0
    int next_page[4] = {0, 0, 0, 0}; // Track the next page number 
    char line[BUFSIZ]; // Store input into the array called 'line'

//  The number of arguments needs to be 3, as there will be the program itself, in.txt and out.txt
    if(argc != 3) {
        fprintf(stderr, "Error! The number of arguments should be exactly 3, whereas you have provided %d arguments.\n", argc - 1);
        return 1;
    }

//  Open in.txt, deal with the failure case  
    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr, "Error! Failed to open the input file.\n");
        return 1;
    }

//  Open out.txt, deal with the failure case
    FILE *output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error! Failed to open the output file.\n");
        return 1;
    }

//  Initialise virtual memory
    initialise_virtual_memory();

//  Initialise RAM
    initialise_RAM();

//  Initialise page table
    initialise_page_table();

//  Read every line from input.txt
    if (fgets(line, sizeof(line), input_file) != NULL) {
        char *pointer = line;
        int process_id; // process_id declaration

        while (sscanf(pointer, "%d", &process_id) == 1) {
            process_id_storage[count++] = process_id; // Process id storage
//  Move the pointer forward to the next integer
            while (*pointer != '\0' && *pointer != ' ') {
                pointer++;
            }
//  Skip spaces between numbers
            while (*pointer == ' ') {
                pointer++;
            }
        }
    }
    fclose(input_file); // Reading is finished now, close the file 

    for (int i = 0; i < count; i++) {
//  Assign the current process id
    int pid = process_id_storage[i];
//  Assign the next page number to this process
    int next_page_number = next_page[pid];
//  Determine whether all pages are in RAM
    bool all_pages_in_RAM = true; // Initialise to true

    for (int page_number = 0; page_number < 4; page_number++) {
        if (page_table[pid][page_number] == 99) { // 99 means the page is in disc
            all_pages_in_RAM = false;
            break;
        }
    }

    if (all_pages_in_RAM) {
//  Ignore if all the pages of a process is already in memory
        continue; 
    } else {
//  If the page is not in the disc, i.e. in RAM
        if (page_table[pid][next_page_number] != 99) {
            int frame = page_table[pid][next_page_number];
//  Update the last access time of a page that is in RAM
            RAM[frame * 2]->last_accessed = time;
//  If the page is not in RAM, i.e. in the disc
        } else {
            int frame = find_free_frame_otherwise_evict(pid, time);
//  Load the page from virtual memory into RAM
            bring_pages_from_virtual_memory_into_RAM(pid, next_page_number, frame, time);
//  Update the next page number for the process. As each process has 4 pages, the "% 4" can make sure the page number cycles properly
            next_page[pid] = (next_page[pid] + 1) % 4; 
        }
    }
    time++; 
    }    

//  Write the page table to output file
    for (int pid = 0; pid < 4; pid++) {
        for (int page_number = 0; page_number < 4; page_number++) {
//  Write the frame number so that the program will know whether a page is in RAM or disc
            fprintf(output_file, "%d", page_table[pid][page_number]);
//  Page number ranges from 0 to 3, 3 is the last number, < 3 ensures no extra comma is printed after the last page number
            if (page_number < 3) {
                fprintf(output_file, ", ");
            }  
        } 
        fprintf(output_file, "\n");
    } 
    
    fprintf(output_file, "\n");
 
//  Write the RAM content to output file
    for (int i = 0; i < 16; i++) {
        if (RAM[i] != NULL) {
            fprintf(output_file, "%d,%d,%d", RAM[i]->process_id, RAM[i]->page_number, RAM[i]->last_accessed);
//  If the slot is empty, print "NULL"
        } else {
            fprintf(output_file, "NULL");
        } 
//  No semicolon is printed after the last entry
        if (i < 15) {
           fprintf(output_file, "; ");
        }
    }
    fprintf(output_file, "\n");
    fclose(output_file);

//  Free allocated memory
    for (int i = 0; i < 32; i += 2) {
        if (virtual_memory[i] != NULL) {
            free(virtual_memory[i]);
            virtual_memory[i] = NULL; 
            virtual_memory[i + 1] = NULL;
        }
    }
    return 0;
}






