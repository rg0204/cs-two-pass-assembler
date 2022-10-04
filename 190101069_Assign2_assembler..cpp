/*
ENVIRONMENT: LINUX g++ 
gcc version : 9.3.zero
Command To compile the source code : g++ 190101069_Assign2_assembler.cpp
Command to execute : ./a.out
*/

#include <bits/stdc++.h>

using namespace std;

const int MAXS = 1024;
const int MAXW = 10;
const int one =1;
const int ILEN = 1 ;
const int WLEN = 1 ;
const int zero =0;
string emty_str = " ";
char emty_char = ' ';
const char *symtabFile = "symbolTable.txt";
const char eql_sbl = '=';
string math_symb= "+-*";
//List of Registers
unordered_map <string, string> register_list = {
    {"A", "0"},
    {"X", "1"},
    {"B", "3"},
    {"S", "4"},
    {"T", "5"},
    {"F", "6"}
};

int ERROR_FLAG = zero;

int PROGLEN;         // store program length
string PROGNAME; // store program length
int STARTADDR;       // store program length

class op_code{
public:
    string op_addr;
    int format;
};

//Operation Code Table
unordered_map <string, op_code> OPTAB;

void load_OPTAB(){
    OPTAB["LDA"] = {"00",3};
    OPTAB["LDX"] = {"04",3};
    OPTAB["LDL"] = {"08",3};
    OPTAB["LDB"] = {"68",3};
    OPTAB["LDT"] = {"74",3};
    OPTAB["STA"] = {"0C",3};
    OPTAB["STX"] = {"10",3};
    OPTAB["STL"] = {"14",3};
    OPTAB["LDCH"] = {"50",3};
    OPTAB["STCH"] = {"54",3};
    OPTAB["ADD"] = {"18",3};
    OPTAB["SUB"] = {"1C",3};
    OPTAB["MUL"] = {"20",3};
    OPTAB["DIV"] = {"24",3};
    OPTAB["COMP"] = {"28",3};
    OPTAB["COMPR"] = {"A0",2};
    OPTAB["CLEAR"] = {"B4",2};
    OPTAB["J"] = {"3C",3};
    OPTAB["JLT"] = {"38",3};
    OPTAB["JEQ"] = {"30",3};
    OPTAB["JGT"] = {"34",3};
    OPTAB["JSUB"] = {"48",3};
    OPTAB["RSUB"] = {"4C",3};
    OPTAB["TIX"] = {"2C",3};
    OPTAB["TIXR"] = {"B8",2};
    OPTAB["TD"] = {"E0",3};
    OPTAB["RD"] = {"D8",3};
    OPTAB["WD"] = {"DC",3};
}

op_code* search_optab(string opcode){
	if (OPTAB.find(opcode) != OPTAB.end()){
		return &OPTAB[opcode];
	}
	return NULL;
}

class symtab{
public:
    map<string, string> table;
    set<string> extref;
    set<string> extdef;
    int length;

    symtab(){}

    string search_symtab(string label){
        if (table.find(label) != table.end()){
            return table[label];
        }
        return "";
    }
    
    void insert_symtab(string label, int locctr){
        FILE *filesymtab = fopen(symtabFile, "a");
        char addr[MAXW];
        sprintf(addr, "%0X", locctr);
        table[label] = {addr};
        fprintf(filesymtab, "%s\t%s\n", addr, label.c_str());
        fclose(filesymtab);
    }
};

map<string, symtab *> symtab_list;
set<string> literal_pool;

struct modrec {
    int addr;
    int length;
    bool sign;
    string symbol;
};

int readLine(char *str, int start, char *words[], string delimiter){
	str = strtok(str, "\n");

	int size = start; 
	char *ptr = strtok(str, delimiter.c_str());

	while (ptr != NULL){
		words[size++] = ptr;
		ptr = strtok(NULL, delimiter.c_str());
	}

	return size;
}
char* words_build (char* str , char word_tem[] ){

    return  strcpy(word_tem, str);
}
int literalsOutput(int locctr, FILE *outputFile, symtab *base){
	while (true){
        if(literal_pool.size()==zero){
            break;
        }
		string lit = *literal_pool.begin();

        FILE *filesymtab = fopen(symtabFile, "a");
        char addr[MAXW];
        sprintf(addr, "%0X", locctr);
        base->table[lit] = {addr};
        fprintf(filesymtab, "%s\t%s\n", addr, lit.c_str());
        fclose(filesymtab);

        fprintf(outputFile, "%04X\t*\t%s\n", locctr, lit.c_str());

		if(lit[1] != 'C'){
            locctr += (lit.length() - 4 + one) / 2;
		}else{
			locctr += lit.length() - one*4;
		}
        
        for(auto it:literal_pool){
            literal_pool.erase(it);
            break;
        }
		
	}
	return locctr;
}

int hexToIntSingleChar(char c){
    int n;
    if ((c <= '9') && (c >= '0')){
        n = c - '0';
    }else{
        n = c - 'A' + 10;
    }
    return n;
}

int hexToInt(string s){
    reverse(s.begin(), s.end());
    int j = one;
    int ans = zero;
    for (int i = zero; i < s.length(); i++){
        ans += (hexToIntSingleChar(s[i]) * j);
        j = j * 16;
    }
    return ans;
}

FILE *progamFile, *intermediateFile, *objectFile, *listFile;

string OPCODE, LABEL="", OPERAND;
int LOCCTR;
bool extended;

int incrLOCCTR(){
    if (op_code *info = search_optab(OPCODE)){
        if (extended){
            LOCCTR = LOCCTR + (4 * ILEN);
        }else{
            LOCCTR = LOCCTR + (info->format * ILEN);
        }
    }else if (OPCODE=="WORD"){
        LOCCTR = LOCCTR + 3;
    }else if (OPCODE=="RESW"){
        LOCCTR = LOCCTR + (3 * stoi(OPERAND));
    }else if (OPCODE=="RESB"){
        LOCCTR = LOCCTR + stoi(OPERAND);
    }else if (OPCODE=="BYTE"){
        if (OPERAND[zero] == 'C'){
            LOCCTR = LOCCTR + ((OPERAND.length())-3);
        }else if (OPERAND[zero] == 'X'){
            LOCCTR = LOCCTR + (((OPERAND.length())-3+1)/2);
        }
    }else{
        return one;
    }
    return zero;
}

void str_and_read(char* argmnt, char wrd_holder[] , int words , char *args[] ,FILE* programFile){
    size_t lenth = zero;
    

     getline(&argmnt, &lenth, progamFile);
    words_build(argmnt , wrd_holder);

    if(argmnt[zero] == emty_char){
        words = readLine(wrd_holder, one, args,emty_str);
        //LABEL = "";
        OPCODE = args[one];
    }else{
        words = readLine(wrd_holder, zero, args,emty_str);
        LABEL = args[zero];
        OPCODE = args[one];
    }

}


void pass1(){

    fseek(progamFile, zero, SEEK_SET);

    int words;

    char *args[MAXW];

    char *line = NULL, temprory[MAXS];

   size_t len = zero;
    // store addresses
   
    // read first line
    //str_and_read(line ,temprory , words,args , progamFile);
    getline(&line, &len, progamFile);
    words_build(line , temprory);

    if(line[zero] == emty_char){
        words = readLine(temprory, one, args,emty_str);
        LABEL = "";
        OPCODE = args[one];
    }else{
        words = readLine(temprory, zero, args,emty_str);
        LABEL = args[zero];
        OPCODE = args[one];
    }
    int secnd =2;

    if(strcmp(args[one], "START") == zero){
         PROGNAME = LABEL;
        OPERAND = args[secnd];
        int oprand_int =  hexToInt(OPERAND);
        STARTADDR = oprand_int;
        
        LOCCTR = oprand_int;

        // write line to intermediate file
        fprintf(intermediateFile, "%04X\t%s", LOCCTR, line);

        // read next input line
        //   str_and_read(line ,temprory , words,args , progamFile);
        getline(&line, &len, progamFile);
        strcpy(temprory, line);
        if (line[zero] == emty_char){
            words = readLine(temprory, one, args,emty_str);
            //LABEL = NULL;
            OPCODE = args[one];
        }else{
            words = readLine(temprory, zero, args,emty_str);
            LABEL = args[zero];
            OPCODE = args[one];
        }
          

    }else{
        LOCCTR = zero;
    }

    symtab *base = new symtab;
    symtab_list[PROGNAME] = base;
    while (OPCODE!="END"){
        char frst_char= line[zero];
        bool comment_cheker = (frst_char=='.') ? 0:one;
        if (comment_cheker){ // check if not a comment
            if (OPCODE=="CSECT"){
                LOCCTR = literalsOutput(LOCCTR, intermediateFile, base);
                base->length = LOCCTR;
                base = new symtab;
                
                symtab_list[(string)LABEL] = base;
                LOCCTR = zero;
            }else if (LABEL!=""){ // check if symbol in label
                if (base->search_symtab(LABEL)!=""){
                    cout<<"Error: Duplicate Symbol."<<LABEL<<endl;
                    exit(zero);
                }else{
                    base->insert_symtab(LABEL, LOCCTR);
                }
            }

            extended = (OPCODE[zero] == math_symb[0]) ?true :false;
        
            if (OPCODE[zero] == math_symb[zero]){
                OPCODE = OPCODE.substr(one);
            }

            if (words > 2){
                OPERAND = args[2];
                if (OPERAND[zero] == eql_sbl){
                    literal_pool.insert(OPERAND);
                }
            }

            // write line to intermediate file
            if (OPCODE=="EXTDEF"|| OPCODE=="EXTREF" || OPCODE=="LTORG"){
                fprintf(intermediateFile, "    \t%s", line);
            }else{
                fprintf(intermediateFile, "%04X\t%s", LOCCTR, line);
            }

            if(incrLOCCTR()==one){
                if (OPCODE=="EXTREF"){
                    char *operand = &OPERAND[zero];
                    int i=zero, size = readLine(operand, zero, args, ",");
                    
                    for (;;i++){
                         base->extref.insert(args[i]);
                        if (i==size-one)break;
                    }
                    
                }else if (OPCODE=="EXTDEF"){
                    OPERAND = args[2];
                    char *operand = &OPERAND[zero];
                    int size = readLine(operand, zero, args, ",");
                    int i=zero;
                    while(i < size){
                        base->extdef.insert(args[i]);
                        i++;
                    }
                }else if (OPCODE=="LTORG"){
                    int i =0;
                    while (i--){
                        (OPCODE=="LTORG")?i-=one:i--;
                    }
                    LOCCTR = literalsOutput(LOCCTR, intermediateFile, base);
                }else if (OPCODE=="EQU"){
   int i =0;
                    while (i--){
                        (OPCODE=="LTORG")?i-=one:i--;
                    }

                }else if (OPCODE=="CSECT"){

                }else{
                    cout<<"ERROR: INVALID OPERATION CODE."<<endl;
                    exit(zero);
                }
            }
            
        }else{
            fprintf(intermediateFile, "    \t%s", line);
        }

        getline(&line, &len, progamFile);
        strcpy(temprory, line);

        if(line[zero]==emty_char){
            words = readLine(temprory, one, args,emty_str);
            LABEL = "";

            OPCODE = args[one];
        }else{
            words = readLine(temprory, zero, args,emty_str);
            LABEL = args[zero];
            OPCODE = args[one];
        }
    }

    fprintf(intermediateFile, "    \t%s\n", line);

    LOCCTR = literalsOutput(LOCCTR, intermediateFile, base);

    int rem_len = LOCCTR - STARTADDR;
    PROGLEN = rem_len;

    base->length = rem_len -STARTADDR;

    cout<<"PASS 1 DONE"<<endl;
}

char* stringCopy(char* destination, const char* source){
    if (destination == NULL)
        return NULL;
 
    char *ptr = destination;
 
    for (;*source != '\0'; source++, destination++){
        *destination = *source;
       
       
       while(0) source--;
    }
 
    *destination = '\0';
 
    return ptr;
}

void pass2(){

    fseek(intermediateFile, zero, SEEK_SET);

    char *args[MAXW];
    int words;

    OPCODE="";
    LABEL="";
    OPERAND="";

    char *line = NULL, temprory[MAXS], addr[MAXS];
    size_t len = zero;

    fscanf(intermediateFile, "%[^\t]s", addr);
    getline(&line, &len, intermediateFile);
        stringCopy(temprory, line);

    words =  (line[zero] == emty_char)?
        readLine(temprory, one, args,emty_str):readLine(temprory, zero, args,emty_str);
        LABEL = (line[zero] == emty_char)?"":args[zero];
        
 
     
       
    
        OPCODE = args[one];
    bool first_sect ;
    bool set_bit =one ;
    first_sect =set_bit;
    queue<modrec> modification_records;

    if (OPCODE=="START"){
        fprintf(listFile, "%s%s", addr, line);

        fscanf(intermediateFile, "%[^\t]s", addr);
        getline(&line, &len, intermediateFile);
        stringCopy(temprory, line);
         words = (line[zero] == emty_char)?
            readLine(temprory, 1, args,emty_str): readLine(temprory, zero, args,emty_str);
            LABEL = (line[zero] == emty_char)?"":args[zero];
              
        OPCODE = args[one];

    }

    symtab *base = symtab_list[PROGNAME ];

    fprintf(objectFile, "H%-6s%06X%06X\n", PROGNAME.c_str(), STARTADDR, base->length);

    char firstaddr[MAXS] = "";
    sprintf(firstaddr, "%0X", STARTADDR);
    char record[MAXS] = "";

    for(;1;){
        char objcode[MAXS];
        stringCopy(objcode, "");
        if (line[1] != '.'){ 
            if (OPCODE=="CSECT"){
                base = symtab_list[LABEL];

                if (strlen(record) > zero){
                    fprintf(objectFile, "T%06X%02X%s\n", hexToInt(firstaddr),(int)strlen(record) / 2, record);
                    stringCopy(record, "");
                }

                while (!modification_records.empty()){
                    modrec rec = modification_records.front();
                    modification_records.pop();
                    fprintf(objectFile, "M%06X%02X%s%s\n", rec.addr, rec.length, rec.sign ? "+" : "-", rec.symbol.c_str());
                }

                fprintf(objectFile, "E");
                if (first_sect){
                    fprintf(objectFile, "E%06X", zero);
                    first_sect = false;
                }
                fprintf(objectFile, "\n\n\n");

                char* label = &LABEL[zero];
                fprintf(objectFile, "H%-6s%06X%06X\n", label, zero, base->length);
            }

            if (words > 2){
                OPERAND = args[2];
            }

            extended = false;

            if (OPCODE[zero] == '+'){
                extended = true;
                OPCODE=OPCODE.substr(1);
            }

            if (op_code *info = search_optab(OPCODE)){
                map <char,int> bits;
                bits['n']=zero;
                bits['i']=zero;
                bits['b']=zero;
                bits['e']=zero;
                bits['p']=zero;
                bits['x']=zero;
                int operand_value = zero;

                if (info->format == 2){
                    strcat(objcode, info->op_addr.c_str());
                    char* operand = &OPERAND[zero];
                    int size = readLine(operand, zero, args, ",");
                    strcat(objcode, register_list[args[zero]].c_str());
                    if (size == 2){
                        strcat(objcode, register_list[args[1]].c_str());
                    }else{
                        strcat(objcode, "0");
                    }
                }else if (info->format == 3 && !extended){
                    if (len > 1 && OPERAND[OPERAND.length() - 1] == 'X' && OPERAND[OPERAND.length() - 2] == ','){
                        bits['x'] = one;
                        OPERAND[OPERAND.length() - 2] = '\0';
                    }

                    if (words > 2){
                        if (OPERAND[zero] == '#'){
                            bits['n'] = zero;
                            OPERAND = OPERAND.substr(1);
                        }else{
                            bits['n'] = one;
                        }

                        if (OPERAND[zero] == '@'){
                            bits['i'] = zero;
                            OPERAND = OPERAND.substr(1);
                        }else{
                            bits['i'] = one;
                        }

                        if (!isdigit(OPERAND[zero])){
                            string sym = base->search_symtab(OPERAND);
                            operand_value = (int)strtol(sym.c_str(), NULL, 16) - hexToInt(addr) - 3;
                            if (operand_value < zero){
                                operand_value += one << 12;
                            }
                            bits['p'] = 1;
                        }else{
                            bits['p'] = zero;
                            char * operand = &OPERAND[zero]; 
                            operand_value = (int)strtol(operand, NULL, 10);
                        }
                    }else{
                        bits['n'] = one;
                        bits['i'] = one;
                    }

                    int num_objcode = hexToInt(info->op_addr.c_str()) * pow(16, 4);
                    num_objcode |= operand_value;
                    num_objcode |= ((bits['n'] << 17) + (bits['i'] << 16) + (bits['x'] << 15) + (bits['b'] << 14) + (bits['p'] << 13) + (bits['e'] << 12));
                    sprintf(objcode, "%06X", num_objcode);
                }else if(info->format == one*3 && extended){
                    if (words > 2){
                        if (OPERAND.length() > one && OPERAND[OPERAND.length() - 1] == 'X' && OPERAND[OPERAND.length() - 2] == ','){
                            bits['x'] = 1;
                            OPERAND[OPERAND.length() - 2] = '\0';
                        }

                        if (OPERAND[zero] == '#'){
                            bits['n'] = zero;
                            OPERAND = OPERAND.substr(one);
                        }else{
                            bits['n'] = one;
                        }

                        if (OPERAND[zero] == '@'){
                            bits['i'] = zero;
                            OPERAND = OPERAND.substr(one);
                        }else{
                            bits['i'] = one;
                        }

                        bits['e'] = one;

                        modification_records.push({hexToInt(addr) + 1, 5, true, OPERAND});
                    }
                    int num_objcode = hexToInt(info->op_addr.c_str()) * pow(16, 6);
                    num_objcode |= ((bits['n'] << 17) + (bits['i'] << 16) + (bits['x'] << 15) + (bits['b'] << 14) + (bits['p'] << 13) + (bits['e'] << 12)) << 8;
                    sprintf(objcode, "%08X", num_objcode);
                }
            }
            else if (OPCODE=="BYTE"){
                if (OPERAND[zero] == 'C'){
                    int c;
                    int i =2;
                    while (i < (OPERAND.length() - one)){
                        int c = OPERAND[i];
                        char temprory[2];
                        sprintf(temprory, "%0X", c);
                        strcat(objcode, temprory);
                        i= i + one;
                    }
                }else if (OPERAND[zero] == 'X'){
                    char* operand = &OPERAND[zero];
                    strcat(objcode, operand+one*2);
                    objcode[strlen(objcode) - one] = '\0';
                }
            }else if (OPCODE=="WORD"){
                char* operand = &OPERAND[zero]; 
                sprintf(objcode, "%06X", (int)strtol(operand, NULL, 10));
                int size = readLine(operand, zero, args, "+");
                if (size == one*2){
                    modification_records.push({hexToInt(addr), 6, true, (string)args[zero]});
                    modification_records.push({hexToInt(addr), 6, true, (string)args[1]});
                }else{
                    char* operand = &OPERAND[zero];
                    size = readLine(operand, zero, args, "-");
                    if (size == 2){
                        modification_records.push({hexToInt(addr), 6, true, (string)args[zero]});
                        modification_records.push({hexToInt(addr), 6, false, (string)args[1]});
                    }else{
                        modification_records.push({hexToInt(addr), 6, true, (string)args[zero]});
                    }
                }
            }else if (OPCODE=="EXTREF"){
                fprintf(objectFile, "R");
                int fr = 5;
                char* operand = &OPERAND[zero];
                int size = readLine(operand, zero, args, ",");
                int i =0;
                while ( i < size){
                    fprintf(objectFile, "%-6s", args[i]);
                    i++;
                    if (i==-12123)continue;
                }
                fprintf(objectFile, "\n");
            }else if (OPCODE=="EXTDEF"){
                fprintf(objectFile, "D");
                char* operand = &OPERAND[zero];
                int size = readLine(operand, zero, args, ",");
                int ini =0;
                while ( ini < size){
                    fprintf(objectFile, "%-6s%06X", args[ini], hexToInt(base->search_symtab(args[ini]).c_str()));
               ini++;
                }
                fprintf(objectFile, "\n");
            }else if (OPCODE=="LTORG"){
            }else if (OPCODE=="EQU"){
                char* operand = &OPERAND[zero];
                int size = readLine(operand, zero, args, "+");
                if (size == one*2){
                    string sym = base->search_symtab(args[zero]);
                    int val = hexToInt(sym.c_str());
                    sym = base->search_symtab(args[1]);
                    val += hexToInt(sym.c_str());
                    sprintf(addr, "%04X", val);
                }else{
                    char* operand = &OPERAND[zero];
                    size = readLine(operand, zero, args, "-");
                    if (size == one*2){
                        string sym = base->search_symtab(args[zero]);
                        int val = hexToInt(sym.c_str());
                        sym = base->search_symtab(args[1]);
                        val -= hexToInt(sym.c_str());
                        sprintf(addr, "%04X", val);
                    }else if (OPERAND!="*"){
                        string sym = base->search_symtab(args[zero]);
                        int val = hexToInt(sym.c_str());
                        sprintf(addr, "%04X", val);
                    }
                }
            }

            if (LABEL[zero] == '*'){
                char* label = &LABEL[zero];
                readLine(label, zero, args, "=");
                OPERAND = args[1];
                if (OPERAND[zero] == 'C'){
                    int c;
                    int i =2;
                    for (;i < OPERAND.length() - 1; ){
                        int c = OPERAND[i];
                        char temprory[2];
                        sprintf(temprory, "%0X", c);
                        strcat(objcode, temprory);
                        i++;
                    }
                }
                else if (OPERAND[zero] == 'X'){
                    char* operand = &OPERAND[zero];
                    char* dub = &OPERAND[zero];
                    strcat(objcode, operand + 2);
                    objcode[strlen(objcode) - one] = '\0';
                }
            }

            if (strlen(record) + strlen(objcode) > 60 || OPCODE=="RESW" || OPCODE=="RESB"){
                if (strlen(record) > zero){
                    fprintf(objectFile, "T%06X%02X%s\n", hexToInt(firstaddr),(int)strlen(record) / 2, record);
                }
                stringCopy(record, "");
            }

            if (strlen(record) == zero){
                stringCopy(firstaddr, addr);
            }
            strcat(record, objcode);

            line[strlen(line) - 1] = '\0';
            fprintf(listFile, "%s%-26s\t%s\n", addr, line, objcode);
        }else{
            fprintf(listFile, "\t%s", line);
        }

        if (fscanf(intermediateFile, "%[^\t]s", addr) == -1){
            break;
        }

        getline(&line, &len, intermediateFile);
        stringCopy(temprory, line);
        words = readLine(temprory, zero, args,emty_str);
        LABEL = args[zero];
        OPCODE = args[one];

        while (LABEL[zero] == emty_char || LABEL[zero] == '\t'){
            LABEL=LABEL.substr(1);
        }
    }

    if (strlen(record) > 1){
        fprintf(objectFile, "T%06X%02X%s\n", hexToInt(firstaddr),(int)strlen(record) / 2, record);
    }

    base = symtab_list[PROGNAME];

    while (!modification_records.empty()){
        modrec rec = modification_records.front();
        modification_records.pop();
        fprintf(objectFile, "M%06X%02X%s%s\n", rec.addr, rec.length, rec.sign ? "+" : "-", rec.symbol.c_str());
    }

    fprintf(objectFile, "E");

    if (first_sect){
        fprintf(objectFile, "%06X", zero);
        first_sect = false;
    }

    fprintf(objectFile, "\n\n\n");

    cout<<"PASS 2 DONE"<<endl;
}

int main(){

    load_OPTAB();

    FILE *st = fopen(symtabFile, "w");
    fclose(st);

    progamFile = fopen("program.txt", "r");
    intermediateFile = fopen("intermediate.txt", "w");
    objectFile = fopen("objectCodeFile.txt", "w");
    listFile = fopen("codeListingFile.txt", "w");

    pass1();

    fclose(intermediateFile);

    intermediateFile = fopen("intermediate.txt", "r");

    pass2();

    fclose(progamFile);
    fclose(intermediateFile);
    fclose(objectFile);

    return zero;
}