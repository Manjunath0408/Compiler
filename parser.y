%{
    #include "index.h"

    int yylex();
	void yyerror(string);
	void Assembly();
	// get from lex file.
	char mytext[10000];

	// Pointer to the Absract Syntax Tree
	ASTNode* AST;  
	
	int Num_variables=0;

	map<pair<string,string>, int> symbol_table;
	map<string,int> list_size;
	
	// map<string,pair<int,int>> matrix;

	vector<map<string,string>> all_variable_types;
	map<string,string> variable_types;
	vector<map<pair<string,string>, int>> all_symbol_tables;
	
	map<string,int> function_scope;
	map<string,int> function_args; 
	
	void dotraversal(ASTNode* head);
	
	vector<string> text;
	vector<string> data;
	vector<string> bss; 
	vector<string> printint;
	vector<string> printList;
	vector<string> printNewLine;
	map<string,vector<string>> function_arguments;
	map<string,int> Num_variablesF;


	void CodeGenerator(ASTNode* root);
	void putx86inafile();
	void set_integer_print_subroutine();
	void set_data_segment();
	void set_scanner_integer();
	void string_to_number_subroutine();
	void set_list_print_subroutine();
	void set_go_to_new_line_subroutine();
	int load_into_register(string);
	void makenulls();
	
	int count_loops=0;
	int num_scans=0;

	string scope="";
%}

%union{
    class ASTNode* node;
}

%token INT STRING CHARACTER FOR WHILE PRINT SCAN ARRAY RETURN
%token EE LE GE GT LT NE IF ELSE OR AND NOT BOR BAND BXOR COMMA SEMICOLON
%token LSB RSB RCB LCB LNB RNB

%token NEG_INT INC IC POS_INT  CHARACTER_CONSTANT DEC STRING_CONSTANT FUNCTION_IDENTIFIER ELIF IDENTIFIER

%left 	PLUS
%left 	MULTIPLY
%left 	MINUS
%left 	DIVIDE
%right 	EQUALTO

%type<node> PROGRAM DECLARATIONS DECLARATION VARIABLE_DECLARATION VARIABLE_TYPE 
%type<node> STATEMENTS STATEMENT ASSIGNMENT_STATEMENT IDENTIFIER_NT FUNCTION_IDENTIFIER_NT

%type<node> EXPRESSION PEXPRESSION  ARRAY_ELEMENT  INTEGER_NT STRING_NT CHARACTER_NT FUNCTION_DECLARATION COMPOUND_STATEMENT 

%type<node> LOCAL_DECLARATION 
%type<node> PRINT_STATEMENT SCAN_STATEMENT PRINT_SCAN_ITEM 
%type<node> RETURN_STATEMENT RETURN

%type<node> IF_STATEMENT WHILE_STATEMENT FOR_STATEMENT INCDEC_STATEMENT
%type<node> PARAM PARAMS PARAM_LIST_NT

%type<node> INT STRING CHARACTER ARRAY IC IDENTIFIER FUNCTION_IDENTIFIER NEG_INT POS_INT STRING_CONSTANT CHARACTER_CONSTANT COMMA SEMICOLON LCB RCB LNB RNB PLUS MINUS MULTIPLY DIVIDE
%type<node> IF ELSE ELIF WHILE FOR PRINT SCAN OR AND NOT EQUALTO LT GT LE GE EE NE INC DEC BAND BOR BXOR LSB RSB ARRAY_TYPE 

%%

PROGRAM:	DECLARATIONS 
			{
				$$=new ASTNode("PROGRAM",{$1});
				AST=$$;
			};

DECLARATIONS: 	DECLARATIONS DECLARATION
					{
						$$=new ASTNode("DECLARATIONS",{$1,$2});
					}
				| 	DECLARATION 
					{
						$$=new ASTNode("DECLARATIONS",{$1});
					};

DECLARATION: 	VARIABLE_DECLARATION
				{
					$$=new ASTNode("DECLARATION",{$1});
				}
			| 	FUNCTION_DECLARATION 
				{
					$$=new ASTNode("DECLARATION",{$1});
				};


								
VARIABLE_DECLARATION:  	VARIABLE_TYPE IDENTIFIER_NT SEMICOLON 
						{
							$3=new ASTNode("SEMICOLON");

							$$ = new ASTNode("VARIABLE_DECLARATION", {$1,$2,$3});

							// TODO:
							Num_variables++;

							if(variable_types.find($2->lexValue)==variable_types.end())
							{
								string var_type = $1->children[0]->nodeType;
								symbol_table[{$2->lexValue,var_type}]=Num_variables*-4; 
								// Store the variables in a Map.Key is the name of variable.Value is the address in stack.
								variable_types[$2->lexValue]=var_type;
							}
						}
						|	ARRAY_TYPE IDENTIFIER_NT LSB POS_INT RSB SEMICOLON 
						{
							$3=new ASTNode("LSB");
							$5=new ASTNode("RSB");
							$6=new ASTNode("SEMICOLON");
							$$ = new ASTNode("VARIABLE_DECLARATION", {$1, $2, $3,$4,$5,$6});
							
							Num_variables+=atoi(mytext);
							list_size[$2->lexValue]=atoi(mytext);
							
							if(variable_types.find($2->lexValue)==variable_types.end()){
								string var_type=$1->children[0]->nodeType;
								symbol_table[{$2->lexValue,var_type}]=Num_variables*-4;  // Store the variables in a Map.Key is the name of variable.Value is the address in stack.
								variable_types[$2->lexValue]=var_type;
							}
						};

ARRAY_TYPE: ARRAY
			{
				$1 = new ASTNode("ARRAY");
				$$ = new ASTNode("ARRAY_TYPE",{$1});
			};
										


FUNCTION_DECLARATION: 	VARIABLE_TYPE FUNCTION_IDENTIFIER_NT LNB PARAMS RNB COMPOUND_STATEMENT 
						{
							$3 = new ASTNode("LNB"); 
							$5 = new ASTNode("RNB");

							$$ = new ASTNode("FUNCTION_DECLARATION", {$1, $2, $3, $4, $5, $6});
							function_args[$2->lexValue]=0;
							int tot_num_fun=function_scope.size();
							function_scope[$2->lexValue]=tot_num_fun;
							scope=$2->lexValue;
							all_symbol_tables.push_back(symbol_table);
							all_variable_types.push_back(variable_types);
							
							vector<string> arguments;
							
							ASTNode* params=$4;
							if(params->children.size()!=0 && params->children[0]->nodeType!="EPSILON")
							{
								ASTNode* paramsnt=params->children[0];
								int ch=0;

								while(paramsnt->children.size()>2){
									arguments.push_back(to_string(symbol_table[{paramsnt->children[2]->children[1]->lexValue,"INT"}]));
									ch++;
									paramsnt=paramsnt->children[0];
								}
								ch++;
								arguments.push_back(to_string(symbol_table[{paramsnt->children[0]->children[1]->lexValue,"INT"}]));
								function_args[$2->lexValue]=ch;
							}
							Num_variablesF[$2->lexValue]=Num_variables;
							Num_variables=0;
							reverse(arguments.begin(),arguments.end());
							function_arguments[$2->lexValue]=arguments;
							symbol_table.clear();
							variable_types.clear();
						};



PARAMS: PARAM_LIST_NT {
						vector<ASTNode*> v = {$1};
						$$ = new ASTNode("PARAMS",v);
					}								
		| {
			auto eps= new ASTNode("EPSILON");
			vector<ASTNode*> v = {eps};
			$$ = new ASTNode("PARAMS",v);
		};



PARAM_LIST_NT: PARAM_LIST_NT COMMA PARAM {
										$2 = new ASTNode("COMMA");
										vector<ASTNode*> v = {$1,$2,$3};
										$$ = new ASTNode("PARAM_LIST_NT",v);
									} 
				| PARAM {
					vector<ASTNode*> v = {$1};
					$$ = new ASTNode("PARAM_LIST_NT",v);
				};



PARAM: VARIABLE_TYPE IDENTIFIER_NT{
									Num_variables++;
									string var_type=$1->children[0]->nodeType;
									if(var_type!="INT"){
										yyerror("You can pass only Integer arguments...");
									}
									symbol_table[{$2->lexValue,var_type}]=Num_variables*-4;
									variable_types[$2->lexValue]=var_type;
									vector<ASTNode*> v = {$1,$2};
									$$ = new ASTNode("PARAM",v);
								}
		| IDENTIFIER_NT{
			vector<ASTNode*> v = {$1};
			$$ = new ASTNode("PARAM",v);
		};



STATEMENTS: STATEMENTS STATEMENT {
										vector<ASTNode*> v = {$1, $2};
                                        $$ = new ASTNode("STATEMENTS", v); 
										}
				| STATEMENT {
							vector<ASTNode*> v = {$1};
                            $$ = new ASTNode("STATEMENTS", v); 
							} ;



STATEMENT: 		ASSIGNMENT_STATEMENT 
			{
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STATEMENT", v);
			}
			|	COMPOUND_STATEMENT 
			{
				vector<ASTNode*> v = {$1};
                $$ = new ASTNode("STATEMENT", v);
			} 
			|	IF_STATEMENT 
			{
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STATEMENT",v);
			}
			|	WHILE_STATEMENT 
			{
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STATEMENT",v);
			}
			| 	FOR_STATEMENT 
			{
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STATEMENT",v);
			}
			|	INCDEC_STATEMENT 
			{
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STATEMENT",v);
			}
			|	LOCAL_DECLARATION 
			{
				
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STATEMENT",v);
			}
			|	PRINT_STATEMENT
			{
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STATEMENT",v);
			}
			| SCAN_STATEMENT
			{
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STATEMENT",v);
			}
			| RETURN_STATEMENT
			{
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STATEMENT",v);
			};

RETURN_STATEMENT: RETURN IDENTIFIER_NT SEMICOLON
				{
					$1=new ASTNode("RETURN");
					$3=new ASTNode("SEMICOLON");
					$$ = new ASTNode("RETURN_STATEMENT",{$1,$2,$3});
				}
				| RETURN INTEGER_NT SEMICOLON
				{
					$1=new ASTNode("RETURN");
					$3=new ASTNode("SEMICOLON");
					$$ = new ASTNode("RETURN_STATEMENT",{$1,$2,$3});
				};


PRINT_STATEMENT: PRINT LNB PRINT_SCAN_ITEM RNB SEMICOLON 
				{
					$1 = new ASTNode("PRINT");
					$2 = new ASTNode("LNB");
					$4 = new ASTNode("RNB");
					$5=new ASTNode("SEMICOLON");
					$$ = new ASTNode("PRINT_STATEMENT",{$1,$2,$3,$4,$5});
				};


SCAN_STATEMENT: SCAN LNB PRINT_SCAN_ITEM RNB SEMICOLON 
				{
					$1 = new ASTNode("SCAN");
					$2 = new ASTNode("LNB");
					$4 = new ASTNode("RNB");
					$5=new ASTNode("SEMICOLON");
					$$ = new ASTNode("SCAN_STATEMENT", {$1,$2,$3,$4,$5});
				};



COMPOUND_STATEMENT: LCB STATEMENTS RCB    
					{
						// cout << "here";
						$1 = new ASTNode("LCB");
						$3 = new ASTNode("RCB");
						$$ = new ASTNode("COMPOUND_STATEMENT", {$1, $2, $3});
					};



IF_STATEMENT: 	IF LNB EXPRESSION RNB STATEMENT 
				{
					$1 = new ASTNode("IF");
					$2 = new ASTNode("LNB");
					$4 = new ASTNode("RNB");
					$$ = new ASTNode("IF_STATEMENT",{$1,$2,$3,$4,$5});
				}
				| IF LNB EXPRESSION RNB STATEMENT ELSE STATEMENT
				{
					$1 = new ASTNode("IF");
					$2 = new ASTNode("LNB");
					$4 = new ASTNode("RNB");
					$6 = new ASTNode("STATEMENT");
					$$ = new ASTNode("IF_STATEMENT",{$1,$2,$3,$4,$5,$6,$7});
				};



WHILE_STATEMENT: WHILE LNB EXPRESSION RNB STATEMENT 
				{
					$1 = new ASTNode("WHILE_STATEMENT");
					$2 = new ASTNode("LNB");
					$4 = new ASTNode("RNB");
					$$ = new ASTNode("WHILE_STATEMENT",{$1,$2,$3,$4,$5});
				};




FOR_STATEMENT: FOR LNB ASSIGNMENT_STATEMENT EXPRESSION SEMICOLON INCDEC_STATEMENT RNB STATEMENT {
					$1 = new ASTNode("FOR");
					$2 = new ASTNode("LNB");
					$5 = new ASTNode("SEMICOLON");
					$7 = new ASTNode("RNB");
					vector<ASTNode*> v = {$1,$2,$3,$4,$5,$6,$7,$8};
					$$ = new ASTNode("FOR_STATEMENT",v);				
										};



INCDEC_STATEMENT: 	IDENTIFIER_NT INC SEMICOLON   
					{
						$2 = new ASTNode("INC");
						$3 = new ASTNode("SEMICOLON");
						$$ = new ASTNode("INCDEC_STATEMENT",{$1, $2, $3});
					}
				|	IDENTIFIER_NT DEC SEMICOLON 
					{
						$2 = new ASTNode("DEC");
						$3 = new ASTNode("SEMICOLON");
						$$ = new ASTNode("INCDEC_STATEMENT", {$1, $2, $3});
                	};



VARIABLE_TYPE: INT {
					$1 = new ASTNode("INT");
					$$ = new ASTNode("VARIABLE_TYPE",{$1});

				}
				| STRING{
					$1 = new ASTNode("STRING");
					$$ = new ASTNode("VARIABLE_TYPE",{$1});
				}
				| CHARACTER {
					$1 = new ASTNode("CHARACTER");
					$$ = new ASTNode("VARIABLE_TYPE",{$1});
				};




LOCAL_DECLARATION: 	VARIABLE_TYPE IDENTIFIER_NT SEMICOLON 
					{
													
						$3=new ASTNode("SEMICOLON");
						$$ = new ASTNode("LOCAL_DECLARATION", {$1, $2, $3});
						Num_variables++;
						if(variable_types.find($2->lexValue)==variable_types.end()){
							string var_type=$1->children[0]->nodeType;
							symbol_table[{$2->lexValue,var_type}]=Num_variables*-4;  // Store the variables in a Map.Key is the name of variable.Value is the address in stack.
							variable_types[$2->lexValue]=var_type;
						}
					}
					| ARRAY_TYPE IDENTIFIER_NT LSB INTEGER_NT RSB SEMICOLON 
					{
							$3=new ASTNode("LSB");
							$5=new ASTNode("RSB");
							$6=new ASTNode("SEMICOLON");
							$$ = new ASTNode("LOCAL_DECLARATION", {$1, $2, $3,$4,$5,$6});
							
							Num_variables+=atoi(mytext);
							list_size[$2->lexValue]=atoi(mytext);
							if(variable_types.find($2->lexValue)==variable_types.end())
							{
								string var_type=$1->children[0]->nodeType;
								symbol_table[{$2->lexValue,var_type}]=Num_variables*-4;  // Store the variables in a Map.Key is the name of variable.Value is the address in stack.
								
								variable_types[$2->lexValue]=var_type;
							}
						};



ASSIGNMENT_STATEMENT: 	IDENTIFIER_NT EQUALTO EXPRESSION SEMICOLON 
						{									// Identifier and Expression are given as children to EQUAL TO OPERATOR IN SYNTAX TREE.
							$2=new ASTNode("EQUALTO",{$1,$3});
							$4=new ASTNode("SEMICOLON");
							$$=new ASTNode("ASSIGNMENT_STATEMENT",{$2,$4});
						};



EXPRESSION: PEXPRESSION 
			{	
				$$=new ASTNode("EXPRESSION",{$1});
			}
            |  PEXPRESSION PLUS PEXPRESSION 
			{
                $2=new ASTNode("PLUS",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
            }
            | PEXPRESSION MINUS PEXPRESSION 
			{
                $2=new ASTNode("MINUS",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
            }
            | PEXPRESSION MULTIPLY PEXPRESSION 
			{
               $2=new ASTNode("MULTIPLY",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
            }
			| PEXPRESSION DIVIDE PEXPRESSION 
			{
                $2=new ASTNode("DIVIDE",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
            }
			| PEXPRESSION BAND PEXPRESSION 
			{
               $2=new ASTNode("BAND",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
            }
			| PEXPRESSION BXOR PEXPRESSION 
			{
                $2=new ASTNode("BXOR",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
            }
			| PEXPRESSION BOR PEXPRESSION 
			{
               $2=new ASTNode("BOR",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
            }
			| PEXPRESSION GE PEXPRESSION
			{
				$2=new ASTNode("GE",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
			}
			| PEXPRESSION LE PEXPRESSION
			{
				$2=new ASTNode("LE",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
			}
			| PEXPRESSION GT PEXPRESSION
			{
				$2=new ASTNode("GT",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
			}
			| PEXPRESSION LT PEXPRESSION
			{
				$2=new ASTNode("LT",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
			}
			| PEXPRESSION EE PEXPRESSION
			{
				$2=new ASTNode("EE",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
			}
			| PEXPRESSION NE PEXPRESSION
			{
				$2=new ASTNode("NE",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
			}
			| EXPRESSION AND EXPRESSION
			{
				$2=new ASTNode("AND",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
			}
			| EXPRESSION OR EXPRESSION{
				$2=new ASTNode("OR",{$1,$3});
                $$=new ASTNode("EXPRESSION",{$2});
			}
			| NOT EXPRESSION{
                $2=new ASTNode("NOT",{$2});
                $$=new ASTNode("EXPRESSION",{$1});
			};

ARRAY_ELEMENT: 	ARRAY_ELEMENT INTEGER_NT SEMICOLON 
				{
					$3=new ASTNode("SEMICOLON");
					vector<ASTNode*> v={$1,$2,$3};
					$$=new ASTNode("ARRAY_ELEMENT",v);
				} 
				| INTEGER_NT SEMICOLON 
				{
					$2=new ASTNode("SEMICOLON");
					vector<ASTNode*> v={$1,$2};
					$$=new ASTNode("ARRAY_ELEMENT",v);
				};


PEXPRESSION: FUNCTION_IDENTIFIER_NT LNB PARAMS RNB
			{
				//cout<<"YYYYYY\n";
				$2 = new ASTNode("LNB");
				$4 = new ASTNode("RNB");
				vector<ASTNode*> v={$1,$2,$3,$4};
				$$=new ASTNode("PEXPRESSION",v);
			}
			| 	INTEGER_NT 
				{	
					vector<ASTNode*> v={$1};
					$$=new ASTNode("PEXPRESSION",v);
				}
			| IDENTIFIER_NT {
				vector<ASTNode*> v={$1};
				$$=new ASTNode("PEXPRESSION",v);
			}
			|IDENTIFIER_NT LSB INTEGER_NT  RSB{
				$2 = new ASTNode("LSB"); $4 = new ASTNode("RSB");
				vector<ASTNode*> v={$1,$2,$3,$4};
				$$=new ASTNode("PEXPRESSION",v);
			}
			|IDENTIFIER_NT LSB IDENTIFIER_NT  RSB{
				$2 = new ASTNode("LSB"); $4 = new ASTNode("RSB");
				vector<ASTNode*> v={$1,$2,$3,$4};
				$$=new ASTNode("PEXPRESSION",v);
			}
			| LNB EXPRESSION RNB {
				$1 = new ASTNode("LNB"); $3 = new ASTNode("RNB");
                vector<ASTNode*> v = {$1, $2, $3};
                $$ = new ASTNode("PEXPRESSION", v);
			
			}
			| LSB ARRAY_ELEMENT  RSB {
				$1 = new ASTNode("LSB");
				$3 = new ASTNode("RSB");
				vector<ASTNode*> v={$1,$2,$3};
				$$=new ASTNode("PEXPRESSION",v);
			}
			| CHARACTER_NT{
				vector<ASTNode*> v={$1};
				$$=new ASTNode("PEXPRESSION",v);
			};



PRINT_SCAN_ITEM: IDENTIFIER_NT {
				vector<ASTNode*> v = {$1};
                $$ = new ASTNode("PRINT_SCAN_ITEM", v);
				$$->lexValue=$1->lexValue;
			};


INTEGER_NT: POS_INT{
				//cout<<"&\n";
				$1 = new ASTNode("POS_INT");
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("INTEGER_NT", v);
				$$->lexValue = mytext;
			}
			| NEG_INT {
				//cout<<"!\n";
				$1 = new ASTNode("NEG_INT");
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("INTEGER_NT", v);
				$$->lexValue = mytext;
			} ;

STRING_NT: STRING_CONSTANT{
				$1 = new ASTNode("STRING_CONSTANT");
				vector<ASTNode*> v = {$1};
				$$ = new ASTNode("STRING_NT", v);
				$$->lexValue = mytext;
			};

CHARACTER_NT: CHARACTER_CONSTANT
				{
					$1 = new ASTNode("CHARACTER_CONSTANT");
					vector<ASTNode*> v = {$1};
					$$ = new ASTNode("CHARACTER_NT", v);
					$$->lexValue = mytext;
				};

IDENTIFIER_NT: 	IDENTIFIER 
				{
							$1 = new ASTNode("IDENTIFIER");
                            vector<ASTNode*> NodeList= {$1};
                            $$ = new ASTNode("IDENTIFIER_NT",NodeList);
                            $$->lexValue = mytext;
				}
				|	IDENTIFIER_NT LSB INTEGER_NT  RSB{
							$2 = new ASTNode("LSB"); $4 = new ASTNode("RSB");
							vector<ASTNode*> v={$1,$2,$3,$4};
							$$=new ASTNode("IDENTIFIER_NT",v);
				}
				|IDENTIFIER_NT LSB IDENTIFIER_NT  RSB{
							$2 = new ASTNode("LSB"); $4 = new ASTNode("RSB");
							vector<ASTNode*> v={$1,$2,$3,$4};
							$$=new ASTNode("IDENTIFIER_NT",v);
				};



FUNCTION_IDENTIFIER_NT: FUNCTION_IDENTIFIER
						{
							$1 = new ASTNode("FUNCTION_IDENTIFIER");
                            $$ = new ASTNode("FUNCTION_IDENTIFIER_NT",{$1});
                            $$->lexValue=mytext;
						};

%%

extern FILE *yyin;

int main(){
	yyparse();
	CodeGen(AST); 
	// putx86inafile();
	Assembly();
	return 0;
}
