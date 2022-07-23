#include "index.h"
vector<string> registers;

vector<int> u(10, 0);
int time_lru = 0;
vector<string> regs_replacement;

int least_recently_used()
{
	return *min_element(u.begin() + 4, u.end());
}

int load_into_register(string stack_delta)
{ // load into register. If it is already there , then directly return , if full , replace with lru;
	time_lru++;
	for (int i = 4; i <= 9; i++)
	{
		if (regs_replacement[i] == stack_delta)
		{
			u[i] = time_lru;
			return i;
		}
	}
	if (u[4] == 0 || u[5] == 0 || u[6] == 0 || u[7] == 0 || u[8] == 0 || u[9] == 0)
	{
		// TODO: stack pointer
		text.push_back("move " + registers[3] + " , $fp");
		text.push_back("addi " + registers[3] + " , " + registers[3] + ", " + stack_delta);
		int i;
		for (int j = 4; j <= 9; j++)
		{
			if (u[j] == 0)
			{
				i = j;
				break;
			}
		}
		u[i] = time_lru;
		text.push_back("lw " + registers[i] + ", 0(" + registers[3] + ")");
		regs_replacement[i] = stack_delta;
		return i;
	}
	else
	{
		// TODO: stack pointer
		text.push_back("move " + registers[2] + " , $fp");
		text.push_back("addi " + registers[2] + " , " + registers[2] + "," + stack_delta);
		for (int i = 4; i <= 9; i++)
		{
			if (u[i] == least_recently_used())
			{
				u[i] = time_lru;
				text.push_back("lw " + registers[i] + ", 0(" + registers[2] + ")");
				regs_replacement[i] = stack_delta;
				return i;
			}
		}
	}
}

int store_into_register(string ident)
{
	time_lru++;
	for (int i = 4; i <= 9; i++)
	{
		if (regs_replacement[i] == ident)
		{
			text.push_back("move " + registers[2] + " , $fp");
			text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + ident);
			text.push_back("lw " + registers[i] + ", 0(" + registers[2] + ")");
			u[i] = time_lru;
			return i;
		}
	}
	if (u[4] == 0 || u[5] == 0 || u[6] == 0 || u[7] == 0 || u[8] == 0 || u[9] == 0)
	{
		text.push_back("move " + registers[3] + " , $fp");
		text.push_back("addi " + registers[3] + " , " + registers[3] + " , " + ident);
		int i, j;
		for (int j = 4; j <= 9; j++)
		{
			if (u[j] == 0)
			{
				i = j;
				break;
			}
		}
		u[i] = time_lru;
		text.push_back("lw " + registers[i] + ", 0(" + registers[3] + ")");
		regs_replacement[i] = ident;
		return i;
	}
	else
	{
		text.push_back("move " + registers[2] + " , $fp");
		text.push_back("addi " + registers[2] + " , " + registers[2] + " , " + ident);
		for (int i = 4; i <= 9; i++)
		{
			if (u[i] == least_recently_used())
			{
				u[i] = time_lru;
				text.push_back("lw " + registers[i] + ", 0(" + registers[2] + ")");
				regs_replacement[i] = ident;
				return i;
			}
		}
	}
}
