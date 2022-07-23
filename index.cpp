#include "index.h"

string cur_function_scope = "";
int check_reg = 0;
void CodeGen(ASTNode *root)
{
    if (root->nodeType == "PROGRAM")
    {
        // load all the registers
        registers.push_back("$t0");
        registers.push_back("$t1");
        registers.push_back("$t2");
        registers.push_back("$t3");
        registers.push_back("$t4");
        registers.push_back("$t5");
        registers.push_back("$t6");
        registers.push_back("$t7");
        registers.push_back("$t8");
        registers.push_back("$t9");
        for (int i = 0; i <= 9; i++)
            regs_replacement.push_back("");
        text.push_back(".text");
        text.push_back(".globl main ");
        CodeGen(root->children[0]);
    }
    else if (root->nodeType == "DECLARATIONS")
    {
        CodeGen(root->children[0]);
        if (root->children.size() > 1)
        {
            CodeGen(root->children[1]);
        }
    }
    else if (root->nodeType == "DECLARATION")
    {
        CodeGen(root->children[0]);
    }
    else if (root->nodeType == "LOCAL_DECLARATION")
    {
        if (root->children[0]->nodeType == "VARIABLE_TYPE")
        {
            text.push_back("addi $sp, $sp, -4");
        }
        else if (root->children[0]->nodeType == "ARRAY_TYPE")
        {
            int len = -4 * stoi(root->children[3]->lexValue);
            text.push_back("addi $sp, $sp, " + to_string(len));
        }
    }
    else if (root->nodeType == "FUNCTION_DECLARATION")
    {
        string function_name = root->children[1]->lexValue;
        function_name = function_name.substr(1, function_name.length());
        if (function_name == "main")
        {
            text.push_back("main:");
            // text.push_back("addi $sp, $sp, -4");
            // text.push_back("sw $fp, 0($sp)");
            // text.push_back("addi $sp, $sp, -4");
            text.push_back("move $fp, $sp");
            cur_function_scope = "$main";
            Num_variables = Num_variablesF[cur_function_scope];
            variable_types = all_variable_types[function_scope[cur_function_scope]];
            symbol_table = all_symbol_tables[function_scope[cur_function_scope]];
            CodeGen(root->children[5]);
            text.push_back("move $sp, $fp");
            // text.push_back("addi $sp, $sp, 4");
            // text.push_back("lw $fp, 0($sp)");
            text.push_back("li $v0, 10");
            text.push_back("syscall");
        }
        else
        {
            cur_function_scope = "$" + function_name;
            text.push_back("" + cur_function_scope + ":");
            Num_variables = Num_variablesF[cur_function_scope];
            variable_types = all_variable_types[function_scope[cur_function_scope]];
            symbol_table = all_symbol_tables[function_scope[cur_function_scope]];

            vector<string> args = function_arguments[cur_function_scope];
            int num_args = args.size();
            text.push_back("");
            // text.push_back("sw $fp, 0($sp)");
            // text.push_back("addi $sp, $sp, -4");
            // text.push_back("move $fp, $sp");
            // text.push_back("addi $sp, $sp, " + to_string(-4 * num_args));
            CodeGen(root->children[5]);
            // text.push_back("move $fp, $sp");
            // text.push_back("addi $sp, $sp, 4");
            // text.push_back("lw $fp, 0($sp)");
            text.push_back("jr $ra");
        }
    }
    else if (root->nodeType == "VARIABLE_DECLARATION")
    {
        return;
    }
    else if (root->nodeType == "STATEMENTS")
    {
        CodeGen(root->children[0]);
        if (root->children.size() > 1)
        {
            CodeGen(root->children[1]);
        }
    }
    else if (root->nodeType == "STATEMENT")
    {
        CodeGen(root->children[0]);
    }
    else if (root->nodeType == "COMPOUND_STATEMENT")
    {
        CodeGen(root->children[1]);
    }
    else if (root->nodeType == "PRINT_STATEMENT")
    {
        check_reg = -1;
        if (symbol_table.find({root->children[2]->lexValue, "INT"}) != symbol_table.end())
        {
            text.push_back("");
            string stack_delta = to_string(symbol_table[{root->children[2]->lexValue, "INT"}]);

            text.push_back("move " + registers[1] + ", $fp");
            text.push_back("addi " + registers[1] + ", " + registers[1] + ", " + stack_delta);
            text.push_back("lw " + registers[2] + ", 0(" + registers[1] + ")");
            text.push_back("li $v0, 1");
            text.push_back("move $a0, " + registers[2]);
            text.push_back("syscall");

            text.push_back("");
        }
        else if (symbol_table.find({root->children[2]->lexValue, "CHARACTER"}) != symbol_table.end())
        {
            text.push_back("");
            string stack_delta = to_string(symbol_table[{root->children[2]->lexValue, "CHARACTER"}]);
            text.push_back("move " + registers[1] + " , $fp");
            text.push_back("addi " + registers[1] + ", " + registers[1] + ", " + stack_delta);
            // FIXME:
            text.push_back("lw " + registers[2] + ", 0(" + registers[1] + ")");
            text.push_back("li $v0, 11");
            text.push_back("move $a0, " + registers[2]);
            text.push_back("syscall");
        }
        else if (symbol_table.find({root->children[2]->lexValue, "ARRAY"}) != symbol_table.end())
        {
            text.push_back("");
            int stack_delta = symbol_table[{root->children[2]->lexValue, "ARRAY"}];
            int len = list_size[{root->children[2]->lexValue}];
            stack_delta += 4 * (len - 1);
            while (len > 0)
            {
                text.push_back("move " + registers[1] + " , $fp");
                text.push_back("addi " + registers[1] + ", " + registers[1] + ", " + to_string(stack_delta));
                text.push_back("lw " + registers[0] + ", 0(" + registers[1] + ")");
                text.push_back("li $v0, 1");
                text.push_back("move $a0, " + registers[0]);
                text.push_back("syscall");
                stack_delta -= 4;
                len--;
            }
            text.push_back("");
        }
    }
    else if (root->nodeType == "SCAN_STATEMENT")
    {
        check_reg = -1;
        if (variable_types[root->children[2]->children[0]->lexValue] == "INT")
        {
            text.push_back("li $v0, 5");
            text.push_back("syscall");

            string stack_delta = to_string(symbol_table[{root->children[2]->children[0]->lexValue, "INT"}]);
            text.push_back("move " + registers[0] + ", " + "$v0");
            text.push_back("move " + registers[1] + ", $fp");
            text.push_back("addi " + registers[1] + ", " + registers[1] + ", " + stack_delta);
            text.push_back("sw " + registers[0] + ", 0(" + registers[1] + ")");
        }
    }
    else if (root->nodeType == "INCDEC_STATEMENT")
    {
        if (root->children[1]->nodeType == "INC")
        {
            string stack_delta = to_string(symbol_table[{root->children[0]->lexValue, "INT"}]);
            text.push_back("move " + registers[2] + ", $fp");
            text.push_back("addi " + registers[2] + " , " + registers[2] + ", " + stack_delta);
            text.push_back("lw " + registers[0] + " , 0(" + registers[2] + ")");
            text.push_back("addi " + registers[0] + ", " + registers[0] + ", 1");
            text.push_back("sw " + registers[0] + ", 0(" + registers[2] + ")");
        }
        else if (root->children[1]->nodeType == "DEC")
        {
            string stack_delta = to_string(symbol_table[{root->children[0]->lexValue, "INT"}]);
            text.push_back("move " + registers[2] + ", $fp");
            text.push_back("addi " + registers[2] + " , " + registers[2] + ", " + stack_delta);
            text.push_back("lw " + registers[0] + " , 0(" + registers[2] + ")");
            text.push_back("addi " + registers[0] + ", " + registers[0] + ", -1");
            text.push_back("sw " + registers[0] + ", 0(" + registers[2] + ")");
        }
    }
    else if (root->nodeType == "RETURN_STATEMENT")
    {
        if (root->children[1]->nodeType == "IDENTIFIER_NT")
        {
            if (variable_types[root->children[1]->lexValue] == "INT")
            {

                text.push_back("move " + registers[1] + " , $fp");
                // cout<<to_string(symbol_table[{root->children[1]->lex_val,"INT"}])<<" "<<root->children[1]->lex_val<<" "<<function_scope;
                text.push_back("addi " + registers[1] + " , " + registers[1] + ", " + to_string(symbol_table[{root->children[1]->lexValue, "INT"}]));
                //
                text.push_back("lw $v1 , 0(" + registers[1] + ")");
            }
            else
            {
                string err = "Incorect return type...";
                yyerror(err);
            }
        }
        else if (root->children[1]->nodeType == "INTEGER_NT")
        {
            text.push_back("li $v1 , " + root->children[1]->lexValue);
        }
        else
        {
            string err = "Error at function return statement";
            yyerror(err);
        }
    }
    else if (root->nodeType == "ASSIGNMENT_STATEMENT")
    {
        if (root->children[0]->children[1]->children[0]->nodeType == "PEXPRESSION")
        {
            // IF WE ARE ACCESSING TO ARRAY ELEMENT
            if (root->children[0]->children[1]->children[0]->children.size() == 4 && root->children[0]->children[1]->children[0]->children[0]->nodeType == "IDENTIFIER_NT")
            {
                // ARRAY ELEMENT EQUALS  ARRAY ELEMENT
                ASTNode *identt = root->children[0]->children[1]->children[0]->children[0];
                int sizz = list_size[identt->lexValue];
                // cout<<"HERE\n";
                if (root->children[0]->children[1]->children[0]->children[2]->nodeType == "IDENTIFIER_NT")
                {

                    string req = root->children[0]->children[1]->children[0]->children[2]->lexValue;
                    req = to_string(symbol_table[{req, "INT"}]);
                    text.push_back("move " + registers[0] + " , $fp");

                    text.push_back("addi " + registers[0] + " , " + registers[0] + " , " + req);
                    // length
                    text.push_back("li " + registers[1] + " , " + to_string(sizz));
                    // index
                    text.push_back("lw " + registers[2] + " , 0(" + registers[0] + ")");
                    // length - index
                    text.push_back("sub " + registers[1] + " , " + registers[1] + " , " + registers[2]);
                    // length - index - 1
                    text.push_back("addi  " + registers[1] + "," + registers[1] + " , -1");
                    // text.push_back("mov rax , 8");
                    text.push_back("li " + registers[0] + " , 4");
                    text.push_back("mul " + registers[0] + "," + registers[0] + "," + registers[1]);
                    text.push_back("move " + registers[1] + " , " + registers[0]);
                    // TODO: WTF is this?
                    text.push_back("move " + registers[2] + " , $fp");

                    text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{identt->lexValue, "ARRAY"}]));
                    text.push_back("add " + registers[2] + " , " + registers[2] + " , " + registers[1]);
                    text.push_back("lw " + registers[0] + " , 0(" + registers[2] + ")");
                }
                else if (root->children[0]->children[1]->children[0]->children[2]->nodeType == "INTEGER_NT")
                {
                    int reqq_poss = stoi(root->children[0]->children[1]->children[0]->children[2]->lexValue);
                    // cout<<identt->lexValue<<" "<<sizz<<" "<<reqq_poss<<endl;
                    if (reqq_poss >= 0 && reqq_poss < sizz)
                    {
                        reqq_poss = sizz - reqq_poss - 1;
                        text.push_back("move " + registers[2] + " , $fp");

                        text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{identt->lexValue, "ARRAY"}]));
                        text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string((reqq_poss)*4));
                        text.push_back("lw " + registers[0] + " , 0(" + registers[2] + ")");
                    }
                    else
                    {
                        yyerror("Accessing Incorrect List Index...");
                    }
                }

                if (root->children[0]->children[0]->children.size() == 4)
                {
                    ASTNode *ident = root->children[0]->children[0]->children[0];
                    int siz = list_size[ident->lexValue];
                    if (root->children[0]->children[0]->children[2]->nodeType == "IDENTIFIER_NT")
                    {
                        string req = root->children[0]->children[0]->children[2]->lexValue;
                        req = to_string(symbol_table[{req, "INT"}]);

                        text.push_back("move " + registers[0] + " , $fp");

                        text.push_back("addi " + registers[0] + " , " + registers[0] + " , " + req);
                        text.push_back("li " + registers[1] + " , " + to_string(siz));

                        text.push_back("lw " + registers[2] + " , 0(" + registers[0] + ")");
                        text.push_back("sub " + registers[1] + " , " + registers[1] + " , " + registers[2]);
                        text.push_back("addi " + registers[1] + " , " + registers[1] + " , -1");
                        // text.push_back("mov rax , 8");
                        text.push_back("li " + registers[0] + " , 4");
                        text.push_back("mul " + registers[0] + "," + registers[0] + "," + registers[1]);
                        text.push_back("move " + registers[1] + " , " + registers[0]);
                        // text.push_back("move " + registers[1] + " , rax");

                        text.push_back("move " + registers[2] + " , $fp");

                        text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{ident->lexValue, "ARRAY"}]));
                        text.push_back("add " + registers[2] + " , " + registers[2] + " , " + registers[1]);

                        text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                        // text.push_back("mov [" + registers[2] + "] , " + registers[0] + "");
                    }
                    else
                    {
                        int req_pos = stoi(root->children[0]->children[0]->children[2]->lexValue);
                        // cout<<symbol_table[{ident->lexValue,"ARRAY"}]<<endl;
                        if (root->children[0]->children[0]->children[2]->nodeType == "INTEGER_NT")
                        {
                            if (req_pos >= 0 && req_pos < siz)
                            {

                                req_pos = siz - req_pos - 1;
                                text.push_back("move " + registers[2] + " , $fp");
                                text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{ident->lexValue, "ARRAY"}]));
                                text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string((req_pos)*4));
                                text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                            }
                            else
                            {
                                yyerror("Invalid Index provided to the list...");
                            }
                        }
                        else
                        {
                            //
                        }
                    }
                }
                // ARRAY ELEMENT EQUALS  IDENTIFIER
                else
                {

                    text.push_back("move " + registers[2] + " , $fp");

                    text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{root->children[0]->children[0]->lexValue, "INT"}]));
                    // text.push_back("mov [" + registers[2] + "] , " + registers[0] + "");
                    text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                }
            }
            else if (root->children[0]->children[1]->children[0]->children[0]->nodeType == "INTEGER_NT")
            {
                // IF WE are accessing a particular element from list

                if (root->children[0]->children[0]->children.size() == 4)
                {

                    ASTNode *ident = root->children[0]->children[0]->children[0];
                    int siz = list_size[ident->lexValue];
                    ;
                    if (root->children[0]->children[0]->children[2]->nodeType == "INTEGER_NT")
                    {
                        int req_pos = stoi(root->children[0]->children[0]->children[2]->lexValue);
                        // cout<<"HERE\n";
                        if (req_pos >= 0 && req_pos < siz)
                        {
                            req_pos = siz - req_pos - 1;

                            text.push_back("move " + registers[2] + " , $fp");

                            text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{ident->lexValue, "ARRAY"}]));
                            text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string((req_pos)*4));
                            text.push_back("li " + registers[0] + " , " + root->children[0]->children[1]->children[0]->children[0]->lexValue);
                            text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                        }
                        else
                        {
                            yyerror("Invalid Index provided to the list...");
                        }
                    }
                    else if (root->children[0]->children[0]->children[2]->nodeType == "IDENTIFIER_NT")
                    {
                        // cout<<"uuu\n";
                        string req = root->children[0]->children[0]->children[2]->lexValue;
                        req = to_string(symbol_table[{req, "INT"}]);

                        text.push_back("move " + registers[0] + " , $fp");
                        text.push_back("addi " + registers[0] + " , " + registers[0] + " , " + req);
                        text.push_back("li " + registers[1] + " , " + to_string(siz));
                        text.push_back("lw " + registers[2] + " , 0(" + registers[0] + ")");
                        text.push_back("sub " + registers[1] + " , " + registers[1] + " , " + registers[2]);
                        text.push_back("addi " + registers[1] + "," + registers[1] + " , -1");
                        text.push_back("li " + registers[2] + ", 4");

                        text.push_back("mul " + registers[0] + "," + registers[0] + "," + registers[1]);

                        text.push_back("move " + registers[1] + " , " + registers[0]);
                        // text.push_back("move " + registers[1] + " , rax");

                        text.push_back("move " + registers[2] + " , $fp");

                        text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{ident->lexValue, "ARRAY"}]));
                        text.push_back("add " + registers[2] + " , " + registers[2] + " , " + registers[1]);
                        text.push_back("li " + registers[0] + " , " + root->children[0]->children[1]->children[0]->children[0]->lexValue);
                        text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                    }
                }
                else
                {

                    text.push_back("move " + registers[2] + " , $fp");
                    text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{root->children[0]->children[0]->lexValue, "INT"}]));
                    text.push_back("li " + registers[0] + " , " + root->children[0]->children[1]->children[0]->children[0]->lexValue);
                    text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                }
            }
            else if (root->children[0]->children[1]->children[0]->children[0]->nodeType == "IDENTIFIER_NT")
            {
                // cout<<"IDENTIFIER_NT\n";
                string node_type = variable_types[root->children[0]->children[1]->children[0]->children[0]->lexValue];
                if (node_type == "INT")
                {
                    // IF WE are accessing a particular element from list
                    if (root->children[0]->children[0]->children.size() == 4)
                    {
                        ASTNode *ident = root->children[0]->children[0]->children[0];
                        int siz = list_size[ident->lexValue];
                        if (root->children[0]->children[0]->children[2]->nodeType == "IDENTIFIER_NT")
                        {
                            string req = root->children[0]->children[0]->children[2]->lexValue;
                            req = to_string(symbol_table[{req, "INT"}]);
                            text.push_back("move " + registers[0] + " , $fp");
                            text.push_back("add " + registers[0] + " , " + registers[0] + " , " + req);
                            text.push_back("li " + registers[1] + " , " + to_string(siz));
                            text.push_back("lw " + registers[2] + " , 0(" + registers[0] + ")");
                            text.push_back("sub " + registers[1] + " , " + registers[1] + " , " + registers[2]);
                            text.push_back("addi " + registers[1] + " , -1");
                            text.push_back("li " + registers[0] + ", 4");
                            text.push_back("mul " + registers[0] + "," + registers[0] + "," + registers[1]);
                            // TODO change rax
                            text.push_back("move " + registers[1] + "," + registers[0]);

                            text.push_back("move " + registers[2] + " , $fp");
                            text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{ident->lexValue, "ARRAY"}]));
                            text.push_back("add " + registers[2] + " , " + registers[2] + " , " + registers[1]);
                            text.push_back("move " + registers[3] + " , $fp");
                            text.push_back("addi " + registers[3] + " , " + registers[3] + " , " + to_string(symbol_table[{root->children[0]->children[1]->children[0]->children[0]->lexValue, "INT"}]));
                            text.push_back("lw " + registers[0] + " , 0(" + registers[3] + ")");
                            text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                        }
                        else
                        {
                            int req_pos = stoi(root->children[0]->children[0]->children[2]->lexValue);
                            // cout<<symbol_table[{ident->lexValue,"ARRAY"}]<<endl;
                            if (req_pos >= 0 && req_pos < siz)
                            {
                                req_pos = siz - req_pos - 1;
                                text.push_back("move " + registers[2] + " , $fp");
                                text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{ident->lexValue, "ARRAY"}]));
                                text.push_back("add " + registers[2] + " , " + registers[2] + " , " + to_string((req_pos)*4));

                                text.push_back("move " + registers[3] + " , $fp");
                                text.push_back("addi " + registers[3] + " , " + registers[3] + " , " + to_string(symbol_table[{root->children[0]->children[1]->children[0]->children[0]->lexValue, "INT"}]));
                                text.push_back("lw " + registers[0] + " , 0(" + registers[3] + ")");
                                text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                            }
                            else
                            {
                                yyerror("Invalid Index provided to the list...");
                            }
                        }
                    }
                    else
                    {
                        text.push_back("move " + registers[2] + " , $fp");
                        text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{root->children[0]->children[0]->lexValue, "INT"}]));
                        string ident = to_string(symbol_table[{root->children[0]->children[1]->children[0]->children[0]->lexValue, "INT"}]);
                        int loaded_into = load_into_register(ident);
                        text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                    }
                }
                else
                {
                    //
                }
            }
            else if (variable_types[root->children[0]->children[0]->lexValue] == "ARRAY")
            {

                text.push_back("move " + registers[2] + " , $fp");
                text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{root->children[0]->children[0]->lexValue, "ARRAY"}]));

                ASTNode *elements = root->children[0]->children[1]->children[0]->children[1];
                while (elements->children.size() > 2)
                {
                    string val_from_end = elements->children[1]->lexValue;
                    // cout<<val_from_end<<endl;
                    text.push_back("li " + registers[0] + " , " + elements->children[1]->lexValue);
                    text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
                    text.push_back("addi " + registers[2] + "," + registers[2] + " , 4");
                    elements = elements->children[0];
                }
                string val_from_end = elements->children[0]->lexValue;
                // cout<<val_from_end<<endl;
                text.push_back("li " + registers[0] + " , " + elements->children[0]->lexValue);
                text.push_back("sw " + registers[0] + " , " + " 0(" + registers[2] + ")");
            }
            else if (root->children[0]->children[1]->children[0]->children[0]->nodeType == "FUNCTION_IDENTIFIER_NT")
            {
                text.push_back("addi $sp, $sp, -4");
                text.push_back("sw $fp, 0($sp)");
                text.push_back("move $a3, $sp");
                ASTNode *pexp = root->children[0]->children[1]->children[0];
                if (function_args.find(pexp->children[0]->lexValue) != function_args.end())
                {
                    int ch = 0;
                    ASTNode *params = pexp->children[2];
                    // TODO: change epsilon
                    if (params->children[0]->nodeType == "EPSILON")
                    {
                        ch = 0;
                    }
                    else
                    {
                        ASTNode *paramsnt = params->children[0];

                        while (paramsnt->children.size() > 1)
                        {
                            ch++;
                            text.push_back("addi $sp, $sp, -4");
                            string var_name = paramsnt->children[2]->children[0]->lexValue;
                            string stack_delta = to_string(symbol_table[{paramsnt->children[2]->children[0]->lexValue, "INT"}]);
                            // this loads the value of the parameter into the registers[loaded_into]
                            int loaded_into = load_into_register(stack_delta);
                            text.push_back("sw " + registers[loaded_into] + ", 0($sp)");
                            paramsnt = paramsnt->children[0];
                        }
                        text.push_back("addi $sp, $sp, -4");
                        string var_name = paramsnt->children[0]->children[0]->lexValue;
                        string stack_delta = to_string(symbol_table[{paramsnt->children[0]->children[0]->lexValue, "INT"}]);
                        int loaded_into = load_into_register(stack_delta);
                        text.push_back("sw " + registers[loaded_into] + ", 0($sp)");
                        ch++;
                    }
                    // params stored now check if the number of params matches
                    if (ch == function_args[pexp->children[0]->lexValue])
                    {
                        text.push_back("li " + registers[0] + ", " + to_string(ch));
                        check_reg = -1;

                        text.push_back("move $fp, $a3");
                        text.push_back("jal " + pexp->children[0]->lexValue);

                        text.push_back("move $sp, $fp");
                        text.push_back("lw $fp, 0($sp)");
                        text.push_back("addi $sp, $sp, 4");
                        text.push_back("move " + registers[2] + ", $fp");
                        text.push_back("addi " + registers[2] + ", " + to_string(symbol_table[{root->children[0]->children[0]->lexValue, "INT"}]));
                        text.push_back("sw $v1, 0(" + registers[2] + ")");
                        // TODO: fix stack pointer
                        // text.push_back("addi $sp, $sp, " + to_string(4 * ch));
                    }
                    else
                    {
                        text.push_back("addi $sp , $sp " + to_string(4 * ch));
                        string err = "Check the name of function or the number of arguments";
                        yyerror(err);
                    }
                }
                else
                {
                    string err = "No Function found..";
                    yyerror(err);
                }
            }
            else if (root->children[0]->children[1]->children[0]->children[0]->nodeType == "CHARACTER_NT")
            {
                // TODO:
                text.push_back("move " + registers[2] + " , $fp");
                text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{root->children[0]->children[0]->lexValue, "CHARACTER"}]));
                // TODO:
                text.push_back("mov byte[rcx] , " + root->children[0]->children[1]->children[0]->children[0]->lexValue);
            }
            else
            {
                string err = "Error Occured...";
                yyerror(err);
            }
        }
        else
        {
            // TODO: the whole thing
            // PLUS/MINUS/MULTIPLY/BAND/BOR/BXOR NAMES are done between floats / ints / matrices / lists , come here
            string node_type = variable_types[root->children[0]->children[1]->children[0]->children[0]->children[0]->lexValue];
            string node_type2 = root->children[0]->children[1]->children[0]->children[0]->children[0]->nodeType;
            if (node_type == "ARRAY")
            {
                CodeGen(root->children[0]->children[1]);
                int top_of_stack = Num_variables;
                top_of_stack++;
                top_of_stack *= -4;
                ASTNode *left_list = root->children[0]->children[0];
                int left_list_size = list_size[left_list->lexValue];
                int left_list_loc = symbol_table[{left_list->lexValue, "ARRAY"}];
                int number_of_times = left_list_size;

                text.push_back("move " + registers[3] + " , $fp");
                text.push_back("addi " + registers[3] + " , " + registers[3] + " , " + to_string(left_list_loc));
                text.push_back("move " + registers[2] + " , $fp");
                text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(top_of_stack));
                while (number_of_times > 0)
                {
                    text.push_back("lw " + registers[0] + " , 0(" + registers[2] + ")");
                    text.push_back("sw " + registers[0] + " , 0(" + registers[3] + ")");
                    text.push_back("addi " + registers[3] + " , " + registers[3] + " , " + " 4");
                    text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + " -4");
                    number_of_times--;
                }
            }

            else if (node_type2 == "INTEGER_NT" || node_type == "INT")
            {
                CodeGen(root->children[0]->children[1]);
                text.push_back("move " + registers[2] + " , $fp");
                text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + to_string(symbol_table[{root->children[0]->children[0]->lexValue, "INT"}]));
                text.push_back("sw " + registers[0] + " , 0(" + registers[2] + ")");
                int k;
                k = store_into_register(to_string(symbol_table[{root->children[0]->children[0]->lexValue, "INT"}]));
            }
        }
    }
    else if (root->nodeType == "IF_STATEMENT")
    {
        if (root->children.size() == 5)
        {
            count_loops++;
            string LabelIf = "LabelIf" + to_string(count_loops);
            string EndIf = "EndIf" + to_string(count_loops);
            string NextSkip = "NextSkip" + to_string(count_loops);
            text.push_back(LabelIf + ":");

            ASTNode *condition_root = root->children[2]->children[0];
            string x, y, xx;
            string a, b;

            // left_expr is a PEXP or EXP
            ASTNode *left_expr = condition_root->children[0];
            a = left_expr->children[0]->nodeType;

            // They are dealing with EXP type only ig and that too joined by RELOP only
            if (a == "NEQ")
            {
                x = "beq";
                xx = "bne";
            }
            if (a == "GE")
            {
                x = "blt";
                xx = "bge";
            }
            if (a == "LE")
            {
                x = "bgt";
                xx = "ble";
            }
            if (a == "GT")
            {
                x = "ble";
                xx = "bgt";
            }
            if (a == "LT")
            {
                x = "bge";
                xx = "blt";
            }
            if (a == "EE")
            {
                x = "bne";
                xx = "beq";
            }
            if (condition_root->nodeType == "NOT")
            {
                CodeGen(root->children[2]);
                text.push_back(xx + " " + EndIf);
                CodeGen(root->children[4]);
                text.push_back(EndIf + ":");
                return;
            }

            // right_expr is a PEXP or EXP
            ASTNode *right_expr = condition_root->children[1];
            b = right_expr->children[0]->nodeType;
            if (b == "NEQ")
            {
                y = "beq";
            }
            if (b == "GE")
            {
                y = "blt";
            }
            if (b == "LE")
            {
                y = "bgt";
            }
            if (b == "GT")
            {
                y = "ble";
            }
            if (b == "LT")
            {
                y = "bge";
            }
            if (b == "EE")
            {
                y = "bne";
            }
            if (condition_root->nodeType == "AND" || condition_root->nodeType == "OR")
            {
                string typ = condition_root->nodeType;
                if (typ == "AND")
                {
                    CodeGen(left_expr);
                    text.push_back(x + " " + EndIf);
                    CodeGen(right_expr);
                    text.push_back(y + " " + EndIf);
                }
                else if (typ == "OR")
                {
                    CodeGen(left_expr);
                    text.push_back(xx + " " + NextSkip);
                    CodeGen(right_expr);
                    text.push_back(y + " " + EndIf);
                    text.push_back(NextSkip + ":");
                }
                CodeGen(root->children[4]);
                text.push_back(EndIf + ":");
                return;
            }
            else
            {
                if (condition_root->nodeType == "GE")
                {
                    CodeGen(root->children[2]);
                    text.push_back("blt " + registers[3] + ", $zero ," + EndIf);
                }
                if (condition_root->nodeType == "LE")
                {
                    CodeGen(root->children[2]);
                    text.push_back("bgt " + registers[3] + ", $zero ," + EndIf);
                }
                if (condition_root->nodeType == "GT")
                {
                    CodeGen(root->children[2]);
                    text.push_back("ble " + registers[3] + ", $zero ," + EndIf);
                }
                if (condition_root->nodeType == "LT")
                {
                    CodeGen(root->children[2]);
                    text.push_back("bge " + registers[3] + ", $zero ," + EndIf);
                }
                if (condition_root->nodeType == "EE")
                {
                    CodeGen(root->children[2]);
                    text.push_back("bne " + registers[3] + ", $zero ," + EndIf);
                }
                if (condition_root->nodeType == "NEQ")
                {
                    CodeGen(root->children[2]);
                    text.push_back("beq " + registers[3] + ", $zero ," + EndIf);
                }
            }
            CodeGen(root->children[4]);
            text.push_back(EndIf + ":");
        }
        else if (root->children.size() == 7)
        {
            count_loops++;
            string LabelIf = "LabelIf" + to_string(count_loops);
            string LabelElse = "LabelElse" + to_string(count_loops);
            string EndIf = "EndIf" + to_string(count_loops);
            string EndElse = "EndElse" + to_string(count_loops);
            string NextSkip = "NextSkip" + to_string(count_loops);
            text.push_back(LabelIf + ":");

            ASTNode *condition_root = root->children[2]->children[0];
            string x, y, xx;
            string a, b;

            ASTNode *left_expr = condition_root->children[0];
            a = left_expr->children[0]->nodeType;
            if (a == "NEQ")
            {
                x = "beq";
                xx = "bne";
            }
            if (a == "GE")
            {
                x = "blt";
                xx = "bge";
            }
            if (a == "LE")
            {
                x = "bgt";
                xx = "ble";
            }
            if (a == "GT")
            {
                x = "ble";
                xx = "bgt";
            }
            if (a == "LT")
            {
                x = "bge";
                xx = "blt";
            }
            if (a == "EE")
            {
                x = "bne";
                xx = "beq";
            }
            if (condition_root->nodeType == "NOT")
            {
                CodeGen(root->children[2]);
                text.push_back(xx + " " + EndIf);
                CodeGen(root->children[4]);
                text.push_back(EndIf + ":");
                return;
            }

            ASTNode *right_expr = condition_root->children[1];
            b = right_expr->children[0]->nodeType;
            if (b == "NEQ")
            {
                y = "beq";
            }
            if (b == "GE")
            {
                y = "blt";
            }
            if (b == "LE")
            {
                y = "bgt";
            }
            if (b == "GT")
            {
                y = "ble";
            }
            if (b == "LT")
            {
                y = "bge";
            }
            if (b == "EE")
            {
                y = "bne";
            }

            // For each should add the comparison registers
            if (condition_root->nodeType == "AND" || condition_root->nodeType == "OR")
            {
                string typ = condition_root->nodeType;

                if (typ == "AND")
                {
                    CodeGen(left_expr);
                    text.push_back(x + " " + LabelElse);
                    CodeGen(right_expr);
                    text.push_back(y + " " + LabelElse);
                }
                else if (typ == "OR")
                {
                    CodeGen(left_expr);
                    text.push_back(xx + " " + NextSkip);
                    CodeGen(right_expr);
                    text.push_back(y + " " + LabelElse);
                    text.push_back(NextSkip + ":");
                }

                CodeGen(root->children[4]);
                text.push_back("b " + EndElse);
                text.push_back(LabelElse + ":");
                CodeGen(root->children[6]);
                text.push_back(EndElse + ":");
                return;
            }
            else
            {
                if (condition_root->nodeType == "GE")
                {
                    CodeGen(root->children[2]);
                    text.push_back("blt " + registers[3] + ", $zero ," + LabelElse);
                }
                if (condition_root->nodeType == "LE")
                {
                    CodeGen(root->children[2]);
                    text.push_back("bgt " + registers[3] + ", $zero ," + LabelElse);
                }
                if (condition_root->nodeType == "GT")
                {
                    CodeGen(root->children[2]);
                    text.push_back("ble " + registers[3] + ", $zero ," + LabelElse);
                }
                if (condition_root->nodeType == "LT")
                {
                    CodeGen(root->children[2]);
                    text.push_back("bge " + registers[3] + ", $zero ," + LabelElse);
                }
                if (condition_root->nodeType == "EE")
                {
                    CodeGen(root->children[2]);
                    text.push_back("bne " + registers[3] + ", $zero ," + LabelElse);
                }
                if (condition_root->nodeType == "NEQ")
                {
                    CodeGen(root->children[2]);
                    text.push_back("beq " + registers[3] + ", $zero ," + LabelElse);
                }
            }
            CodeGen(root->children[4]);
            text.push_back("b " + EndElse);
            text.push_back(LabelElse + ":");
            CodeGen(root->children[6]);
            text.push_back(EndElse + ":");
        }
    }
    else if (root->nodeType == "FOR_STATEMENT")
    {
        count_loops++;
        CodeGen(root->children[2]);
        string LabelFor = "LabelFor" + to_string(count_loops);
        string EndFor = "EndFor" + to_string(count_loops);
        string NextSkip = "NextSkip" + to_string(count_loops);
        text.push_back(LabelFor + ":");

        ASTNode *condition_root = root->children[3]->children[0];
        string x, y, xx;
        string a, b;

        ASTNode *left_expr = condition_root->children[0];
        a = left_expr->children[0]->nodeType;

        // using registers[4] for comparison
        if (a == "NEQ")
        {
            x = "beq";
            xx = "bne";
        }
        if (a == "GE")
        {
            x = "blt";
            xx = "bge";
        }
        if (a == "LE")
        {
            x = "bgt";
            xx = "ble";
        }
        if (a == "GT")
        {
            x = "ble";
            xx = "bgt";
        }
        if (a == "LT")
        {
            x = "bge";
            xx = "blt";
        }
        if (a == "EE")
        {
            x = "bne";
            xx = "beq";
        }
        if (condition_root->nodeType == "NOT")
        {
            CodeGen(root->children[2]);
            text.push_back(xx + " " + registers[3] + ", $zero ," + EndFor);
            CodeGen(root->children[7]);
            CodeGen(root->children[5]);
            text.push_back("b " + LabelFor);
            text.push_back(EndFor + ":");
            return;
        }

        ASTNode *right_expr = condition_root->children[1];
        b = right_expr->children[0]->nodeType;
        if (b == "NEQ")
        {
            y = "beq";
        }
        if (b == "GE")
        {
            y = "blt";
        }
        if (b == "LE")
        {
            y = "bgt";
        }
        if (b == "GT")
        {
            y = "ble";
        }
        if (b == "LT")
        {
            y = "bge";
        }
        if (b == "EE")
        {
            y = "bne";
        }

        if (condition_root->nodeType == "AND" || condition_root->nodeType == "OR")
        {
            string typ = condition_root->nodeType;

            if (typ == "AND")
            {
                CodeGen(left_expr);
                text.push_back(x + " " + registers[3] + ", $zero ," + EndFor);
                CodeGen(right_expr);
                text.push_back(y + " " + registers[3] + ", $zero ," + EndFor);
            }
            else if (typ == "OR")
            {
                CodeGen(left_expr);
                text.push_back(xx + " " + registers[3] + ", $zero ," + NextSkip);
                CodeGen(right_expr);
                text.push_back(y + " " + registers[3] + ", $zero ," + EndFor);
                text.push_back(NextSkip + ":");
            }
            CodeGen(root->children[7]);
            CodeGen(root->children[5]);
            text.push_back("b " + LabelFor);
            text.push_back(EndFor + ":");
            return;
        }
        else
        {
            if (condition_root->nodeType == "GE")
            {
                CodeGen(root->children[3]);
                text.push_back("blt " + registers[3] + ", $zero ," + EndFor);
            }
            if (condition_root->nodeType == "LE")
            {
                CodeGen(root->children[3]);
                text.push_back("bgt " + registers[3] + ", $zero ," + EndFor);
            }
            if (condition_root->nodeType == "GT")
            {
                CodeGen(root->children[3]);
                text.push_back("ble " + registers[3] + ", $zero ," + EndFor);
            }
            if (condition_root->nodeType == "LT")
            {
                CodeGen(root->children[3]);
                text.push_back("bge " + registers[3] + ", $zero ," + EndFor);
            }
            if (condition_root->nodeType == "EE")
            {
                CodeGen(root->children[3]);
                text.push_back("bne " + registers[3] + ", $zero ," + EndFor);
            }
            if (condition_root->nodeType == "NEQ")
            {
                CodeGen(root->children[3]);
                text.push_back("beq " + registers[3] + ", $zero ," + EndFor);
            }
        }
        CodeGen(root->children[7]);
        CodeGen(root->children[5]);
        text.push_back("b " + LabelFor);
        text.push_back(EndFor + ":");
    }
    else if (root->nodeType == "WHILE_STATEMENT")
    {
        count_loops++;
        string LabelWhile = "LabelWhile" + to_string(count_loops);
        string EndWhile = "EndWhile" + to_string(count_loops);
        string NextSkip = "NextSkip" + to_string(count_loops);
        text.push_back(LabelWhile + ":");

        ASTNode *condition_root = root->children[2]->children[0];
        string x, y, xx;
        string a, b;

        ASTNode *left_expr = condition_root->children[0];
        a = left_expr->children[0]->nodeType;
        if (a == "NEQ")
        {
            x = "beq";
            xx = "bne";
        }
        if (a == "GE")
        {
            x = "blt";
            xx = "bge";
        }
        if (a == "LE")
        {
            x = "bgt";
            xx = "ble";
        }
        if (a == "GT")
        {
            x = "ble";
            xx = "bgt";
        }
        if (a == "LT")
        {
            x = "bge";
            xx = "blt";
        }
        if (a == "EE")
        {
            x = "bne";
            xx = "beq";
        }
        if (condition_root->nodeType == "NOT")
        {
            CodeGen(root->children[2]);
            text.push_back(xx + " " + registers[3] + ", $zero ," + EndWhile);
            CodeGen(root->children[4]);
            text.push_back("b " + LabelWhile);
            text.push_back(EndWhile + ":");
            return;
        }

        ASTNode *right_expr = condition_root->children[1];
        b = right_expr->children[0]->nodeType;
        if (b == "NEQ")
        {
            y = "beq";
        }
        if (b == "GE")
        {
            y = "blt";
        }
        if (b == "LE")
        {
            y = "bgt";
        }
        if (b == "GT")
        {
            y = "ble";
        }
        if (b == "LT")
        {
            y = "bge";
        }
        if (b == "EE")
        {
            y = "bne";
        }
        if (condition_root->nodeType == "AND" || condition_root->nodeType == "OR")
        {
            string typ = condition_root->nodeType;
            if (typ == "AND")
            {
                CodeGen(left_expr);

                text.push_back(x + " " + registers[3] + ", $zero ," + EndWhile);
                CodeGen(right_expr);
                text.push_back(y + " " + registers[3] + ", $zero ," + EndWhile);
            }
            else if (typ == "OR")
            {
                CodeGen(left_expr);
                text.push_back(xx + " " + registers[3] + ", $zero ," + NextSkip);
                CodeGen(right_expr);
                text.push_back(y + " " + registers[3] + ", $zero ," + EndWhile);
                text.push_back(NextSkip + ":");
            }
            CodeGen(root->children[4]);
            text.push_back("b " + LabelWhile);
            text.push_back(EndWhile + ":");
            return;
        }
        else
        {
            if (condition_root->nodeType == "GE")
            {
                CodeGen(root->children[2]);
                text.push_back("blt " + registers[3] + ", $zero ," + EndWhile);
            }
            if (condition_root->nodeType == "LE")
            {
                CodeGen(root->children[2]);
                text.push_back("bgt " + registers[3] + ", $zero ," + EndWhile);
            }
            if (condition_root->nodeType == "GT")
            {
                CodeGen(root->children[2]);
                text.push_back("ble " + registers[3] + ", $zero ," + EndWhile);
            }
            if (condition_root->nodeType == "LT")
            {
                CodeGen(root->children[2]);
                text.push_back("bge " + registers[3] + ", $zero ," + EndWhile);
            }
            if (condition_root->nodeType == "EE")
            {
                CodeGen(root->children[2]);
                text.push_back("bne " + registers[3] + ", $zero ," + EndWhile);
            }
            if (condition_root->nodeType == "NEQ")
            {
                CodeGen(root->children[2]);
                text.push_back("beq " + registers[3] + ", $zero ," + EndWhile);
            }
        }
        CodeGen(root->children[4]);
        text.push_back("b " + LabelWhile);
        text.push_back(EndWhile + ":");
    }
    else if (root->nodeType == "EXPRESSION")
    {
        // if (root->children[0]->children[0]->nodeType == "EXPRESSION")
        // {
        // }
        string nodeType = variable_types[root->children[0]->children[0]->children[0]->lexValue];
        string node_ident = root->children[0]->children[0]->children[0]->nodeType;

        if (nodeType == "INT" || node_ident == "INTEGER_NT")
        {
            if (root->children[0]->children[0]->nodeType == "PEXPRESSION")
            {
                if (node_ident == "IDENTIFIER_NT")
                {
                    string ident = to_string(symbol_table[{root->children[0]->children[0]->children[0]->lexValue, "INT"}]);
                    int loaded_into = load_into_register(ident);
                    text.push_back("move " + registers[0] + " ," + registers[loaded_into]);
                }
                else if (node_ident == "INTEGER_NT")
                {
                    ////cout<<root->children[0]->children[0]->children[0]->lexValue<<endl;
                    text.push_back("li " + registers[2] + " , " + root->children[0]->children[0]->children[0]->lexValue);
                    text.push_back("move " + registers[0] + " , " + registers[2] + "");
                }
                if (root->children[0]->children[1]->children[0]->nodeType == "IDENTIFIER_NT")
                {

                    string ident = to_string(symbol_table[{root->children[0]->children[1]->children[0]->lexValue, "INT"}]);
                    int loaded_into = load_into_register(ident);
                    text.push_back("move " + registers[1] + " ," + registers[loaded_into]);
                }
                else if (root->children[0]->children[1]->children[0]->nodeType == "INTEGER_NT")
                {
                    ////cout<<root->children[0]->children[1]->children[0]->lexValue<<endl;
                    text.push_back("li " + registers[2] + " , " + root->children[0]->children[1]->children[0]->lexValue);
                    text.push_back("move " + registers[1] + " , " + registers[2] + "");
                }
                string typ = root->children[0]->nodeType;

                if (typ == "PLUS")
                {
                    text.push_back("add " + registers[0] + " , " + registers[0] + "," + registers[1] + "");
                }
                else if (typ == "MINUS")
                {
                    text.push_back("sub " + registers[0] + " , " + registers[0] + "," + registers[1] + "");
                }
                else if (typ == "MULTIPLY")
                {
                    text.push_back("mul " + registers[0] + " , " + registers[0] + "," + registers[1] + "");
                    // text.push_back("mul " + registers[1] + "");
                }
                else if (typ == "DIVIDE")
                {
                    text.push_back("div " + registers[0] + "," + registers[1] + "");
                    text.push_back("mflo " + registers[0]);
                    // quotient stored in $low
                }
                else if (typ == "BAND")
                {
                    text.push_back("and " + registers[0] + " , " + registers[0] + "," + registers[1] + "");
                }
                else if (typ == "BOR")
                {
                    text.push_back("or " + registers[0] + " , " + registers[0] + "," + registers[1] + "");
                }
                else if (typ == "BXOR")
                {
                    text.push_back("xor " + registers[0] + " , " + registers[0] + "," + registers[1] + "");
                }
                else if (typ == "GE" || typ == "LE" || typ == "GT" || typ == "LT" || typ == "EE" || typ == "NEQ")
                {
                    // Using registers[3] for comparison value
                    text.push_back("sub " + registers[3] + "," + registers[0] + "," + registers[1]);
                }
            }
        }
    }
    if (check_reg == -1)
    {
        for (int i = 0; i <= 9; i++)
        {
            regs_replacement[i] = "";
        }
        for (int i = 0; i <= 9; i++)
        {
            u[i] = 0;
        }
        check_reg = 0;
    }
}

void Assembly()
{
    ofstream MyFile("gen.asm");
    for (int i = 0; i < text.size(); i++)
    {
        MyFile << text[i] << endl;
    }
    MyFile << endl
           << endl;
    for (int i = 0; i < printint.size(); i++)
    {
        MyFile << printint[i] << endl;
    }
    MyFile << endl
           << endl;
    for (int i = 0; i < data.size(); i++)
    {
        MyFile << data[i] << endl;
    }
    MyFile << endl
           << endl;
    for (int i = 0; i < bss.size(); i++)
    {
        MyFile << bss[i] << endl;
    }
    MyFile << endl;
    /* for(int i=0;i<printList.size();i++){
        MyFile<<printList[i]<<endl;
    }
    for(int i=0;i<printNewLine.size();i++){
        MyFile<<printNewLine[i]<<endl;
    } */
    MyFile.close();
}

void yyerror(string temp)
{
    cout << endl
         << temp << endl
         << endl;
    ;
    cout << "Parsing Terminated...Syntax Error:(" << endl
         << endl;
    ;
    exit(0);
}