/**********************************************************************************
Program cs2123p2.c by Timothy Hennessy
Purpose:	
    This program includes functions from p0 and p1 
    for printing and converting to postfix.
    It adds a new function, evaluatePostfix, that takes the value
    an infix value from a query file which has been converted
    to postfix and stores the evaluation (boolean) into an array.
Command Parameters:
    p2 -c customerFile -q queryFile
Input:
    Customer File:
        Input file stream which contains two types of records:
        CUSTOMER records are followed by zero to many TRAIT records 
        (terminated by either EOF or another CUSTOMER record).
        CUSTOMER szCustomerId szName
          8s           6s        20s (but the name may have blanks)
        TRAIT    szTraitType  szTraitValue
          8s          10s        12s
     Query File:   
        Input file stream contains queries (one per input text line).
        Tokens in the query will be separated by one space.
        Some sample data:
            SMOKING = N AND GENDER = F
            SMOKING = N AND ( EXERCISE = HIKE OR EXERCISE = BIKE )
            GENDER = F AND EXERCISE NOTANY YOGA
            SMOKING = N AND EXERCISE = HIKE OR EXERCISE = BIKE
            ( BOOK = SCIFI )
            ( ( ( BOOK ONLY SCIFI ) ) )
Results:
    Evaluates a postfix query:
    -Returns an Out structure containing a postfix query
    -Returns the evaluated result of the postfix query
    -Returns either 1 or 0 into queryResultM[]
    -Returns applicable warnings 
Returns:
    Evaluate postfix does not return any values functionally; however,
    it returns boolean integer values (0 and 1) via the parameter
    queryResultM[].
Notes:
    1. This program only allows for 30 customers.
    2. A customer may have multiple occurrences of the same trait type.  For example,
       he/she may have multiple EXERCISE traits because he/she enjoys HIKE, 
       BIKE, and TENNIS.
    3. The number of traits for a customer must not exceed 12.
    4. This program uses an array to implement the stack.  It has a maximum of
       MAX_STACK_ELEM elements. 
    5. This program uses an Out array for the resulting postfix expression.
       It has a maximum of MAX_OUT_ITEM elements.
    6. On the command line, specifying p2 -? will provide the usage information.  
       In some unix shells, you will have to type p2 -\?
****************************************************************************************/

/* include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cs2123p2.h"

/* Function from program # 0 */

/************************ printCustomerData *************************************
void printCustomerData(Customer customerM[], int iNumCustomer)
Purpose:
    Prints data from an array of structures of type Customer including the 
	traits associated with each customer record.
Parameters:
    I	Customer customerM[]	Customer structure array
	I	int	     iNumCustomer	Number of elements in customerM[]
Notes:
    -Uses nested for loops to print each customer and their traits
********************************************************************************/
void printCustomerData(Customer customerM[], int iNumCustomer)
{
    int i;
    int j;
    // Print a heading for the list of customers and traits
    printf("ID         Customer Name\n"
		   "                Trait      Value\n");
	
    for (i = 0; i < iNumCustomer; i++)
    {
        // Print the customer information
		printf("%-11s%s\n" 
				, customerM[i].szCustomerId
				, customerM[i].szCustomerName);

        // Print each of the traits
        for (j = 0; j < customerM[i].iNumberOfTraits; j++)
        {
            // Print a trait
			printf("                %-11s%s\n"
					, customerM[i].traitM[j].szTraitType
					, customerM[i].traitM[j].szTraitValue);

        }
    }
}

/* Functions from program # 1 */

/******************** processRemString ***********************************
int processRemString(Stack stack, Out out)
Purpose:
	Will purge entire stack of all operators until the stack is empty.
    The elements purged from the stack will be sent to addOut.
Parameters:
    I/O Stack stack           pointer to a structure of type StackImp
    I/O Out out               pointer to a structure of type OutImp
Returns:
   TRUE  - if there are no errors 
   FALSE - left parenthesis was left on stack with no matching right
           parenthesis found
Notes:
    1. Program iterates through the remaining stack elements until
       it is empty or a left parenthesis is found.  If a left paren-
       thesis is found FALSE is returned to the convertToPostFix function.
    2. If no left parenthesis is found and the entire stack has been
       traversed, then the program returns true. 
**************************************************************************/
int processRemString(Stack stack, Out out)
{
	Element popElement;

	while (!isEmpty(stack))
	{
		popElement = pop(stack);
		
		if (popElement.iCategory == CAT_LPAREN)
			return FALSE;
		
		addOut(out, popElement);		
	} // end while 
	
	return TRUE;
}
/******************** processRightParen ***********************************
int processRightParen(Stack stack, Out out)
Purpose:
	Loop through stack elements until a corresponding left parenthesis is
    found otherwise all stack elements will be popped from stack and
    FALSE will be returned to convertToPostFix function.         
Parameters:
    I/O Stack stack           pointer to a structure of type StackImp
    I/O Out out               pointer to a structure of type OutImp
Returns:
    TRUE  - if a left parenthesis is found on stack
    FALSE - if no left parenthesis is found on stack
Notes:
    - Pops and stores elements in the out array until a left parenthesis
      is popped from the stack.  Loop execution is terminated by the
      return of TRUE to indicate a properly formatted query.
    - If no left parenthesis is found on stack then FALSE is returned.   
**************************************************************************/
int processRightParen(Stack stack, Out out)
{
	Element popElement;            // element popped from stack
	
	// while stack is not empty
	while (!isEmpty(stack))
	{
		popElement = pop(stack);
		
		// checks to see if popped stack element is a left paren
		if (popElement.iCategory == CAT_LPAREN)
			return TRUE;
		
		// popped stack element is not left paren
		// store it in out
		addOut(out, popElement);
		
	} // end while

	// left parenthesis was not found on stack
	return FALSE;
}

/******************** processOperator **************************************
void processOperator(Stack stack, Element newValue, Out out)
Purpose:
	Takes an operator (token stored inside of newValue) and processes it 
    according to the infix to postfix conversion algorithm.           
Parameters:
    I/O Stack stack           pointer to a structure of type StackImp
    I   Element newValue      pointer to a structure of type Element 
    O   Out out               pointer to a structure of type OutImp
Returns:
   n/a
Notes:
    - If operator token (newValue) has higher precedence than the current 
      operator on top of the stack, then the operator token 
      is pushed on top of the stack (becomes new top value).
    - Otherwise the newValue has a lower precedence than the current 
      operator on top of the stack, the stack is popped and the stack 
      operators are sent to addOut until the stack is empty or an 
      operator on the stack has higher precedence than newValue.
**************************************************************************/
void processOperator(Stack stack, Element newValue, Out out)
{
	Element popElement;                // used to store popped value from stack
	Element topStackElement;
	
	while (!isEmpty(stack))
	{
		topStackElement = topElement(stack);
		
		if (newValue.iPrecedence > topStackElement.iPrecedence)
			break;
		else 
		{
		   popElement = pop(stack);
		   addOut(out, popElement);		
		}

	} // end while
	
	push(stack, newValue);
}
/******************** convertToPostFix **************************************
int convertToPostFix(char *pszInfix, Out out)
Purpose:
	Takes a string from a redirected input file containing operands and 
    operators and converts the string into postfix form.
	It populates the out array using the addOut function (provided in the driver).
Parameters:
    I char *pszInfix       pointer to a string from input file
    O Out  out             pointer to a structure of type OutImp that will
	                       store tokens from the input string 
Returns:
    0   - conversion to postfix was successful
	801 - WARN_MISSING_RPAREN
	802 - WARN_MISSING_LPAREN
Notes:
    - Uses a while loop to traverse a line of text until their are no
      more tokens.
**************************************************************************/
int convertToPostFix(char *pszInfix, Out out)
{
	Stack stack = newStack();               // new dynamically allocated structure
	                                        // of type StackImp stored in a pointer
	                                        // named stack
	char *pszRemainingText;                 // stores address returned by getToken
	                                        // which points to next token in string
	                                        // after delimiter 
	char szfromGetToken[MAX_TOKEN];         // stores token from getToken
	Element element;                        // stores szfromGetToken 
	int bValid = FALSE;                     // stores TRUE or FALSE
	
	
	pszRemainingText = getToken(pszInfix, element.szToken, sizeof(element.szToken) - 1);
	
	while(pszRemainingText != NULL)
	{	
		categorize(&element);                           // argument to categorize function
	                                                    // is a pointer to element
		
		// element can now be used to convert to postfix
		// compare element to categories
		// if element stores an operand send it to out
		if (element.iCategory == CAT_OPERAND)
		{
			addOut(out, element);
		}
		// if element stores a left paren push it onto stack
		 else if(element.iCategory == CAT_LPAREN)
		{
			push(stack, element);                     // push element onto stack
		}
		// if element stores an operator (=, NOTANY, ONLY, AND, OR)
		else if (element.iCategory == CAT_OPERATOR)
		{
			processOperator(stack, element, out);      // process operator according
                                                        // to algorithm 
		}
		// if element stores a right paren 
		else if (element.iCategory == CAT_RPAREN)
		{
			// call function to process right paren
			bValid = processRightParen(stack, out);
			
			if (bValid == FALSE)
			{
				// if bValid is FALSE
				// left parenthesis was never found on stack
				freeStack(stack);
				return WARN_MISSING_LPAREN;
			}
		}

		// retrieve next token
		if (pszRemainingText != NULL)
			pszRemainingText = getToken(pszRemainingText, element.szToken, sizeof(element.szToken) - 1);
	} // end while
	
	// end of input string is reached
	// if stack is not empty
	// process remaining string
	bValid = processRemString(stack, out);
	
	freeStack(stack);
	
	// if right parenthesis is missing return warnring
	if (!bValid)
		return WARN_MISSING_RPAREN;	

	// if successful conversion occurred return 0
	return 0;								
}

/* Functions for program # 2 */
/**************************** atLeastOne **********************************
int atLeastOne(Customer *pCustomer, Trait *pTrait)
Purpose:
    Determines whether a specific trait exists for a customer.  If a
    customer has a specified trait, this function returns TRUE, 
    otherwise it returns FALSE.  
Parameters:
    I Customer *pCustomer     One customer structure which also
                             contains traits of that customer.
    I Trait    *pTrait        The trait that we don't want this 
                             customer to have.
Notes:
    if (notAny returns FALSE) then (atLeastOne returns TRUE)
    if (notAny returns TRUE)  then (atLeastOne returns FALSE)

    The only condition required for this function to return TRUE is
    a given traits existence.  We can also use notAny() for this
    function.  If notAny() returns TRUE, the specified trait does
    not exist and atLeastOne returns FALSE.  If notAny() returns
    FALSE, the specified trait exists and atLeastOne returns TRUE. 
Return value:
    TRUE  - customer has specified trait sought by query
    FALSE - customer does not have specified trait 
**************************************************************************/
int atLeastOne(Customer *pCustomer, Trait *pTrait)
{
    if (pCustomer == NULL)
        ErrExit(ERR_ALGORITHM
        , "received a NULL pointer");
	
	return (!notAny(pCustomer, pTrait));
}
/**************************** only **************************************
int only(Customer *pCustomer, Trait *pTrait)
Purpose:
    Determines whether a customer has ONLY a specified trait type
    and trait value. This function returns TRUE or FALSE.  
Parameters:
    I Customer *pCustomer     One customer structure which also
                             contains traits of that customer.
    I Trait    *pTrait        The trait that we don't want this 
                             customer to have.
Notes:
    ONLY determines whether a customer record has a specified trait
    type and trait value.  If the customer has the trait type and
    a corresponding trait value that matches the function will 
    return TRUE unless there are other trait values of the same
    trait type.
    E.g., Query: BOOK SCIFI ONLY
          TRAIT TYPE     TRAIT VALUE
          BOOK           SCIFI
          EXERCISE       YOGA
    The above situation would return TRUE.
    The below example returns FALSE.
          Query: BOOK SCIFI ONLY
          TRAIT TYPE     TRAIT VALUE
          BOOK           SCIFI
          BOOK           TRAVEL
          EXERCISE       YOGA
            
Return value:
    TRUE - customer has specified trait type and trait value and
           does not have any other trait values of that trait type.
    FALSE - customer did not have the trait or has more than one
            trait value of that trait type.
**************************************************************************/
int only(Customer *pCustomer, Trait *pTrait)
{
	int i;
	int iCountOfTraitTypes = 0;               // will store the count of traitTypes
	                                          // in customer trait array that match
	                                          // pTrait 
	
    if (pCustomer == NULL)
        ErrExit(ERR_ALGORITHM
        , "received a NULL pointer");
	
	for (i = 0; i < (pCustomer->iNumberOfTraits); i++)
	{
		// find if customer has pTrait
		if (strcmp(pCustomer->traitM[i].szTraitType, pTrait->szTraitType) == 0)
			iCountOfTraitTypes++;
	}
	
	for (i = 0; i < (pCustomer->iNumberOfTraits); i++)
	{
		if (strcmp(pCustomer->traitM[i].szTraitType, pTrait->szTraitType) == 0)
			if (strcmp(pCustomer->traitM[i].szTraitValue, pTrait->szTraitValue) == 0)
				if(iCountOfTraitTypes == 1)
					return TRUE;
	}
	return FALSE;
}
/******************** evaluatePostFix *******************************************************
void evaluatePostFix(Out out, Customer customerM[], int iNumCustomer, QueryResult resultM[])
Purpose:
	Takes a query that has been converted to postfix and evaluates the query using
    boolean values and returns TRUE or FALSE.  
Parameters:
    I Out         out           Contains a query converted to postfix   
    I Customer    customerM[]   Contains a structure of array's of type Customer
    I int         iNumCustomer  Stores the total number of customers in customerM structure
                                array
    I\O QueryResult resultM[]   Integer array used to store results of query's, returned
                                via parameter list
                    
Returns:
    Functionally, this program does not return any values.  Through the parameter queryResultM
    this function returns boolean values corresponding to TRUE or FALSE.  Each queryResultM[]
    element corresponds to a customerM structure at the same index.
Notes:
    -Uses two for loops nested together resulting in a computational complexity of order n^2.
********************************************************************************************/
void evaluatePostfix(Out out, Customer customerM[], int iNumCustomer, QueryResult resultM[])
{
	Stack stack = newStack();     // used to store operands from Out and resulting boolean values
	Element evalElem;             // stores values used for evaluation
	Element operand1;             // used to store operand elements 
	Element operand2;             // which were popped from the stack
	Trait trait;                  // store trait types and values popped from stack
	int i;                        // used for outer for loop index
	                              // traverses over customerM array
	int j;                        // used for inner for loop index

	for (i = 0; i < iNumCustomer; i++) 
	{
		for (j = 0; j < (out->iOutCount); j++)
		{
		    Element postElem = out->outM[j];        // stores element value from Out structure
		    switch (postElem.iCategory)
		    {
		       case CAT_OPERAND:
		            push(stack, postElem);
		            break;
		       case CAT_OPERATOR:
		          // this string will make it clear that this
		          // element in the stack is a boolean value
		          strcpy(evalElem.szBoolean, "Boolean result of ");
		          strcat(evalElem.szBoolean, postElem.szToken);
				  operand2 = pop(stack);    // should be trait value or boolean value
				  operand1 = pop(stack);    // should be trait type or boolean value             
			     
				 if (strcmp(postElem.szToken, "AND") == 0)
				 { 
					 // use evalElem popped from stack with logical AND
					 // operation and then push the result back on the stack
				    evalElem.bInclude = (operand1.bInclude && operand2.bInclude);

					push(stack, evalElem);
					break;
				 }
				 
				 if (strcmp(postElem.szToken, "OR") == 0)
				 {
					 // use evalElem popped from stack with logical OR
					 // operation and push the result back on the stack
					evalElem.bInclude = (operand1.bInclude || operand2.bInclude);

					push(stack, evalElem);
					break;
				 }
				 // if values popped from stack are not boolean
				 // i.e., value is a trait and trait type
				 strcpy(trait.szTraitValue, operand2.szToken);              
				 strcpy(trait.szTraitType, operand1.szToken);
				 
		         if (strcmp(postElem.szToken, "=") == 0)
			     { 
				    evalElem.bInclude = atLeastOne(&customerM[i], &trait);
					push(stack, evalElem);
				 }
				 else if (strcmp(postElem.szToken, "NOTANY") == 0)
			     { 
				    evalElem.bInclude = notAny(&customerM[i], &trait);
					push(stack, evalElem);
			     }
				 else if (strcmp(postElem.szToken, "ONLY") == 0)
				 {
				    evalElem.bInclude = only(&customerM[i], &trait);
					push(stack, evalElem);
				 }				  
				 break;
			   default: 
			       printf("\t warning improperly formatted query\n");
			} // end switch
		} // end inner for
		// take the result of the query and store it in QueryResult array at current index i
		evalElem = pop(stack);
		resultM[i] = evalElem.bInclude;
	} // end outter for
	  
	  free(stack);
}
