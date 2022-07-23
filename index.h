#include<bits/stdc++.h>
using namespace std;

class ASTNode 
{
        public:
            vector<ASTNode*> children; 
            string nodeType;              
            string lexValue;             
 
            ASTNode(string node_type, vector<ASTNode*> children) 
            {
                this->children = children;
                this->nodeType = node_type;
                this->lexValue="";
            }
 
            ASTNode(string node_type) 
            {
                children.assign(0, NULL);
                this->nodeType = node_type;
                this->lexValue="";
            }
};


extern vector<map<pair<string,string>, int>> all_symbol_tables;
extern map<string,int> function_scope;
extern map<string,vector<string>> function_arguments;
extern map<string,int> Num_variablesF;
extern vector<map<string,string>> all_variable_types;
extern ASTNode* AST;  // Pointer to the Absract Syntax Tree
extern int Num_variables;
extern map<pair<string,string>, int> symbol_table;
extern map<string,int> list_size;
extern map<string,string> variable_types; 
extern void dotraversal(ASTNode* head);
extern vector<string> text;
extern vector<string> data;
extern vector<string> bss; // 
extern vector<string> printint; // To include the print subroutine
extern vector<string> printList;
extern vector<string> printNewLine;
extern int count_loops;
extern int num_scans;
extern vector<int> u;
extern int store_into_register(string ident);

extern vector<string> registers;
extern int check_reg;
extern int time_lru;
extern int check;
extern vector<string> regs_replacement;
extern map<string,int> function_args; 



void CodeGen(ASTNode* root);
void yyerror(string temp);
void set_go_to_new_line_subroutine();
void set_list_print_subroutine();
void set_integer_print_subroutine();
void set_scanner_integer();
void string_to_number_subroutine();
void set_data_segment();
int load_into_register(string);
void makenulls();
int least_recently_used();