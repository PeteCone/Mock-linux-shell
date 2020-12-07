/*Peter Conant Red ID 821518133*/
/*Professor Carroll CS 570*/
/*9/1/2020*/
/*Program1*/
#include <stdio.h>
#include <stdlib.h>
#include "getword.h"

/*Getword.c*/
/*input: a pointer to the beginning of a String array*/
/*output: returns the number of characters  in a word
SIDE EFFECTS: bytes beginning at address e will be overwritten with the word*/
/*Excludes whitespace and preforms special cases for specific chars and situations*/
/*A word is defined as a string of non meta or of anything except space, tab, new line or EOF (End of File)*/
extern int non_meta_meta_flag;

int getword(char *w)
{
	int charCount = 0;
	int v = 1; /*Multiplier that turns the charCount negative if a $ is detected at the beginning of the word*/
	int iochar = getchar();
	int temp;
	int temp2;
	int prevchar = 0;
	int i;
	char *dirChar;
	*w = '\0';
	dirChar = getenv("HOME");
	non_meta_meta_flag = 0;
	
	/*We need a loop to clear whitespaces (i.e. spaces and tabs) before a word*/
	while ((iochar == ' ')||(iochar == '\t'))
	{
		iochar = getchar();
	}
	
	/*End of File Case*/
	if(iochar == EOF)
	{
		*w = '\0';
		return -255;
	}

	/*The first character after whitespace will be the beginning of a word*/
    /* test */
	/*If the first character of a word is ~ then we need to add home 
	directory to the word*/
	
	if(iochar == '~')
	{
		temp = getchar();
		if((temp == '\n')||(temp == '<') || (temp == '>') ||(temp == '|') || (temp == '&') || (temp == ' ') || (temp == '\\')){
			for(i = 0; i < strlen(dirChar); i++)
			{
				*w++ = dirChar[i]; /*dirChar has the directory string*/
				charCount++;
			}
			iochar = temp;	
		}
		else{
			*w++ = (char)iochar;
			charCount++;
			ungetc(temp, stdin);
			return charCount * v;
		}
		
	}

	/*If the first character of a word is $ return a negative character count.*/
	if(iochar == '$')
	{
		v = -1; /*'v' will be applied at the end of the function*/
		iochar = getchar();
		if((iochar == '\n')||(iochar == EOF))/*if the character after $ if a newline or EOF file, return 0 and an empty string array*/
		{
			ungetc(iochar, stdin);
			return 0;
		}
	}
	
	/*Newline is considered its own word and therefore should not be 
	treated in side the word count while loop*/
	if(iochar == '\n')
	{
		*w='\0';
		return 0;
	}
	
	/*Basic Meta Character Check. Basic Meta Characters include >, <, |, &, '<<'. 
	most meta characters can be checked with a simple char comparison*/
	if((iochar == '>') || (iochar == '|') || (iochar == '&'))
	{
		*w++ = iochar;
		*w='\0';
		charCount++;
		return charCount * v;
	}
	/* Special case << requires us to jump forward in the input string.
	< and << share the same first character therefore we can check for
	the double character metacharacter << here.*/
	
	if(iochar == '<')
	{
		*w++ =(char)iochar; /*follow the same idea as the porevios if statment*/
		charCount++;	
		temp = getchar(); /*check the second character and add if nessecary*/
		if((temp) == '<')
		{
			*w++ =(char)temp;
			charCount++;
		}
		else
		{
			ungetc(temp,stdin);
			
		}
		*w = '\0';
		return charCount * v;
	}


	/*Word Count Loop. Counts and assigns chars to the pointer as the pointer increments. Terminates on EOF, space, tab,
	newline or metacharacter(other than '\').*/
	while(iochar != EOF)
	{
		/*Normal word checking.*/
		if((iochar == ' ')||(iochar == '\t'))
		{
			*w = '\0';
			break;
		}

		/*Meta Character '\' treats the next meta character as a normal character. special \\n case requires us to jump ahead one char in the input string*/
		if(iochar == '\\')
		{
			temp = getchar();

			if((temp) == '\n'){ /*\\n special case should be treated as a space*/
				*w = '\0';
				break;
			}
			else if(temp == '<' || temp == '>' || temp == '|' || temp == '&' || temp == ' ' || temp == '\\' || temp == '$'|| temp == '~')
			{
				*w++=(char)temp;
				charCount++;
				iochar = getchar();
				non_meta_meta_flag = 1;
				continue;
			}
			else
			{
				ungetc(temp, stdin);
				iochar = getchar();
			}	
		}
		
		/*Meta Character and Newline ends word*/
		if((iochar == '\n')||(iochar == '<') || (iochar == '>') ||(iochar == '|') || (iochar == '&') || (iochar == ' '))
		{
			ungetc(iochar, stdin);
			break;
		}

		/*After special cases are checked, assign the character to the next point
		in the w string and increase character counter*/
		*w++=(char)iochar;
		charCount++;
		
		if(charCount == 254)/*254 is our max character per word*/
		{
			break;
		}
		prevchar = iochar;
		iochar = getchar();
	}
	
	*w='\0';
		
	/*Apply $ multiplier*/
	charCount = charCount * v;
	return charCount;
	
}
