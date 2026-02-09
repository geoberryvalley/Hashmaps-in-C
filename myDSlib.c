// CODE 1: Include necessary library(ies)

#include "myDSlib.h"

// -----------------
Record* table = NULL; // DO NOT CHANGE: A global pointer to save the read data from csv files (a pointer to array of Records)
size_t table_size = 0;// DO NOT CHANGE: A global variable showing the number of rows (number of Records) read from csv files (rows in table)
// -----------------

// CODE 2: ADD more global variables if you need
size_t curRow = 0;
// CODE 3: Implement all the functions here

/**
 * Returns the hashed version of a string for use in a hashMap
 * Input:
 *  const char* str - the string to be read
 * Output: 
 *  unsigned int hash - the hash of the string
 */
unsigned int hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c

    return (unsigned int)(hash % INDEX_SIZE);
}

/**
 * Reads a provided british land registry file and stores the data in an array of structs.
 * Each struct in the array corresponds to a row in the table.
 * Input:
 *  const char *fname - the name of the file to read
 * Output: 
 *  None
 */
void read_file(const char *fname){
    //printf("%s column %zu \n",fname,curRow);
    
    FILE *readFile = fopen(fname,"r");
    
    if (readFile == NULL) {
        printf("Error opening file %s. Exiting Program\n", fname);
        exit(1);
    };
    
    char line[MAX_FIELD_LEN*3];
    while(fgets(line, MAX_FIELD_LEN*3, readFile)){
        //printf("%zu\n",curRow);
        //printf("line: %s\n",line);
        //Record new;
        if (curRow == table_size){
            table_size += 100000;
            table = realloc(table, table_size * sizeof(Record));
            if (table == NULL){
                printf("\nFailed to resize table. Exiting Program.\n");
                exit(1);
            };
        };

        int startIndex = 0;//start on the first "
        int endIndex = 1;//end on the ending "
        int curField = 1;
        int lineLength = strlen(line);
        //basically find the start and end indices of the string data we need to copy
        while (endIndex < lineLength){
            while (line[endIndex] != '\"') {
                endIndex++;
            };
            //switch case because it is painful
            switch (curField){
                case 1: //transaction id
                    for (int i = startIndex+1; i<endIndex; i++){//we dont want to include the quotations
                        table[curRow].transaction_id[i-startIndex-1] = line[i];
                    };  
                    break;
                case 2://price
                    table[curRow].price = atoi(&line[startIndex+1]);
                    break;
                case 3://date
                    table[curRow].date.year = atoi(&line[startIndex+1]);
                    table[curRow].date.month = atoi(&line[startIndex+6]);
                    table[curRow].date.day = atoi(&line[startIndex+9]);
                    break;
                case 4: //postcode
                    for (int i = startIndex+1; i<endIndex; i++){//we dont want to include the quotations
                        table[curRow].postcode[i-startIndex-1] = line[i];
                    };
                    break;
                case 5: //property_type
                    table[curRow].property_type = line[startIndex+1];
                    break;
                case 6: //old_new
                    table[curRow].old_new = line[startIndex+1];
                    break;
                case 7: //duration
                    table[curRow].duration = line[startIndex+1];
                    break;
                case 8: //paon
                    for (int i = startIndex+1; i<endIndex; i++){//we dont want to include the quotations
                        table[curRow].paon[i-startIndex-1] = line[i];
                    };  
                    break;
                case 9: //saon
                    for (int i = startIndex+1; i<endIndex; i++){//we dont want to include the quotations
                        table[curRow].saon[i-startIndex-1] = line[i];
                    };  
                    break;
                case 10: //street
                    for (int i = startIndex+1; i<endIndex; i++){//we dont want to include the quotations
                        table[curRow].street[i-startIndex-1] = line[i];
                    };  
                    break;
                case 11: //locality
                    for (int i = startIndex+1; i<endIndex; i++){//we dont want to include the quotations
                        table[curRow].locality[i-startIndex-1] = line[i];
                    };  
                    break;
                case 12: //town
                    for (int i = startIndex+1; i<endIndex; i++){//we dont want to include the quotations
                        table[curRow].town[i-startIndex-1] = line[i];
                    };  
                    break;
                case 13: //district
                    table[curRow].district = calloc(endIndex-startIndex,sizeof(char));
                    if (table[curRow].district == NULL){
                        printf("\nFailed to allocate District value in table. Exiting Program.\n");
                        exit(1);
                    };
                    for (int i = startIndex+1; i<endIndex; i++){//we dont want to include the quotations
                        table[curRow].district[i-startIndex-1] = line[i];
                    };  
                    break;
                case 14: //county
                    for (int i = startIndex+1; i<endIndex; i++){//we dont want to include the quotations
                        table[curRow].county[i-startIndex-1] = line[i];
                    };  
                    break;
                case 15: //record_status
                    table[curRow].record_status = line[startIndex+1];
                    break;
                case 16: //blank_col
                    table[curRow].blank_col = line[startIndex+1];
                    break;
            };
            //update at the end
            startIndex = endIndex+2;//skip over the , to the next "
            endIndex = startIndex+1;//go to the next
            curField++;
        };

        //WE CANT USE STRTOK BECAUSE EMPTY STRINGS AND COMMAS IN STRINGS (GREAT FORMATTING GUYS)
        
        curRow++;
    };
    //reallocate to be more precise at the end
    table_size = curRow;
    table = realloc(table, table_size * sizeof(Record));
    fclose(readFile);
};

/**
 * Creates a hash table from an array of records with keys based on the street name
 * Input: 
 *  Record *tableName - the name of the array to create a hash map from
 *  size_t tableLength - the size of the hash table to be created
 * Output: 
 *  IndexEntry** hashTable - the hash table created
 */
IndexEntry** createIndexOnStreet(Record *tableName, size_t tableLength){
    IndexEntry **hashTable = calloc(tableLength,sizeof(IndexEntry));
    if (hashTable == NULL){
        printf("\nFailed to allocate Hash Table. Exiting Program.\n");
        exit(1);
    };
    unsigned int hash;
    char *streetName;
    for (size_t i = 0; i  < tableLength; i++){
        IndexEntry *new = malloc(sizeof(IndexEntry));
        streetName = tableName[i].street;
        hash = hash_string(streetName);
        new->key=streetName;
        new->record_ptr = &(tableName[i]);
        new->next=NULL;
        if (hashTable[hash] == NULL){
            hashTable[hash] = new;
        } else {
            new->next = hashTable[hash];
            hashTable[hash] = new;
        };
    };
    return hashTable;
};

/**
 * Linearly searches a table of records to find records with the street name matching the target
 * Input: 
 *  Record *tableName - the array to search through
 *  size_t tableLength - the length of the table to search through
 *  const char *streetName - the name of the street to search for
 *  bool printFlag - whether to print the matches or not
 * Output: 
 *  None
 */
void searchStreetLinear(Record *tableName, size_t tableLength, const char *streetName, bool printFlag) {
    for (size_t i = 0; i < tableLength; i++){
        if (strcmp(tableName[i].street,streetName) == 0){
            if (printFlag){
                printf("Match Found: Street = %s | Price = %d | Postcode = %s | Date = %04d-%02d-%02d\n",tableName[i].street,tableName[i].price,tableName[i].postcode,tableName[i].date.year,tableName[i].date.month,tableName[i].date.day);
            } else {
                //do nothing;
            };
        };
    };
};

/**
 * Searches an IndexEntry hash table of records to find records with the street name matching the target
 * Input: 
 *  IndexEntry **indexTableName - the hash table to search through
 *  const char *streetName - the name of the street to search for
 *  bool printFlag - whether to print the matches or not
 * Output: 
 *  None
 */
void searchStreet(IndexEntry **indexTableName, const char *streetName, bool printFlag){
    unsigned int hash = hash_string(streetName);
    IndexEntry *cur = indexTableName[hash];
    while (cur != NULL){
        if (strcmp(streetName, cur->key) == 0 && printFlag){
            printf("Match Found: Street = %s | Price = %d | Postcode = %s | Date = %04d-%02d-%02d\n",cur->record_ptr->street,cur->record_ptr->price,cur->record_ptr->postcode,cur->record_ptr->date.year,cur->record_ptr->date.month,cur->record_ptr->date.day);
        } else {
            //do nothing;
        };
        cur = cur->next;
    };
};

/**
 * Counts the number of unused slots in an IndexEntry hash table
 * Input: 
 *  IndexEntry **indexTableName - the IndexEntry hash table to count through
 * Output: 
 *  int unused - the number of unused slots in the hash table (that point to NULL)
 */
int count_unused_slots(IndexEntry **indexTableName){
    int unused = 0;
    for (int i = 0; i < INDEX_SIZE; i++) {
        if (indexTableName[i] == NULL){
            unused++;
        };
    };
    return unused;
};

/**
 * Frees a hash table of type IndexEntry
 * Input: 
 *  IndexEntry **indexTableName - The hash table to free up
 * Output: 
 *  None
 */
void free_index(IndexEntry **indexTableName){
    IndexEntry *cur;
    IndexEntry *prev;
    for (int i = 0; i < INDEX_SIZE; i++){
        cur = indexTableName[i];
        while (cur != NULL){
            prev = cur;
            cur = cur->next;
            free(prev);
        };
    };
    free(indexTableName);
};

/**
 * Frees the array of records
 * Input: 
 *  None
 * Output: 
 *  None
 */
void free_table(){
    for (size_t i = 0; i < table_size; i++){
        if (table[i].street != NULL){
            free(table[i].district);
        };
    };
    free(table);
};