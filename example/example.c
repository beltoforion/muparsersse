#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _UNICODE
#define _T(x) x
#define myprintf printf
#define mystrlen strlen
#define myfgets fgets
#define mystrcmp strcmp
#else
#define _T(x) L ##x
#define myprintf wprintf
#define mystrlen wcslen
#define myfgets fgetws
#define mystrcmp wcscmp
#endif

#include "muParserSSE.h"

#define PARSER_MAXVARS		10


//---------------------------------------------------------------------------
// Callbacks for postfix operators
mecFloat_t ZeroArg()
{
    myprintf(_T("i'm a function without arguments.\n"));
    return 123;
}

//---------------------------------------------------------------------------
// Callbacks for infix operators
mecFloat_t Not(mecFloat_t v) { return (mecFloat_t)(v == 0); }

//---------------------------------------------------------------------------
// Function callbacks
mecFloat_t Rnd(mecFloat_t v) { return (mecFloat_t)(v * rand() / (double)(RAND_MAX + 1.0)); }


//---------------------------------------------------------------------------
// Binarty operator callbacks
mecFloat_t Add(mecFloat_t v1, mecFloat_t v2)
{
    return v1 + v2;
}

mecFloat_t Mul(mecFloat_t v1, mecFloat_t v2)
{
    return v1*v2;
}

//---------------------------------------------------------------------------
mecFloat_t DebugDump(mecFloat_t v1, mecFloat_t v2)
{
    mecDebugDump((int)v1, (int)v2);
    myprintf(_T("Bytecode dumping %s\n"), ((int)v1 != 0) ? _T("active") : _T("inactive"));
    return 1;
}


//---------------------------------------------------------------------------
void Intro(mecParserHandle_t hParser)
{
    myprintf(_T("                 __________                                       \n"));
    myprintf(_T("    _____   __ __\\______   \\_____  _______  ______  ____ _______\n"));
    myprintf(_T("   /     \\ |  |  \\|     ___/\\__  \\ \\_  __ \\/  ___/_/ __ \\\\_  __ \\ \n"));
    myprintf(_T("  |  Y Y  \\|  |  /|    |     / __ \\_|  | \\/\\___ \\ \\  ___/ |  | \\/ \n"));
    myprintf(_T("  |__|_|  /|____/ |____|    (____  /|__|  /____  > \\___  >|__|    \n"));
    myprintf(_T("        \\/                       \\/            \\/      \\/         \n"));
    myprintf(_T("-------------  Math expression compiler ----------------------\n"));
    myprintf(_T("\n"));
    myprintf(_T("  muParserSSE - V %s\n"), mecGetVersion(hParser));
    myprintf(_T("  (C) 2015 Ingo Berg\n"));
    myprintf(_T("\n"));
    myprintf(_T("--------------------------------------------------------------\n"));
    myprintf(_T("Running test suite:\n"));

#ifndef MEC_DUMP_CMDCODE
    mecSelfTest();
#else
    myprintf(_T("  Unit test skipped\n"));
#endif

    myprintf(_T("--------------------------------------------------------------\n"));
    myprintf(_T("Commands:\n"));
    myprintf(_T("  list var     - list parser variables\n"));
    myprintf(_T("  list exprvar - list expression variables\n"));
    myprintf(_T("  list const   - list all numeric parser constants\n"));
    myprintf(_T("  locale de    - switch to german locale\n"));
    myprintf(_T("  locale en    - switch to english locale\n"));
    myprintf(_T("  locale reset - reset locale\n"));
    myprintf(_T("  quit         - exits the parser\n\n"));
    myprintf(_T("Constants:\n"));
    myprintf(_T("  \"_e\"   2.718281828459045235360287\n"));
    myprintf(_T("  \"_pi\"  3.141592653589793238462643\n"));
    myprintf(_T("--------------------------------------------------------------\n"));
    myprintf(_T("Please enter an expression:\n"));
}

//---------------------------------------------------------------------------
// Callback function for parser errors
void OnError(mecParserHandle_t hParser)
{
    myprintf(_T("\nError:\n"));
    myprintf(_T("------\n"));
    myprintf(_T("Message:  \"%s\"\n"), mecGetErrorMsg(hParser));
    myprintf(_T("Token:    \"%s\"\n"), mecGetErrorToken(hParser));
    myprintf(_T("Position: %d\n"), mecGetErrorPos(hParser));
    myprintf(_T("Errc:     %d\n"), mecGetErrorCode(hParser));
}

//---------------------------------------------------------------------------
void ListVar(mecParserHandle_t a_hParser)
{
    int iNumVar = mecGetVarNum(a_hParser);
    int i = 0;

    if (iNumVar == 0)
    {
        myprintf(_T("No variables defined\n"));
        return;
    }

    myprintf(_T("\nExpression variables:\n"));
    myprintf(_T("---------------------\n"));
    myprintf(_T("Number: %d\n"), iNumVar);

    for (i = 0; i < iNumVar; ++i)
    {
        const mecChar_t* szName = 0;
        mecFloat_t* pVar = 0;

        mecGetVar(a_hParser, i, &szName, &pVar);
        myprintf(_T("Name: %s    Address: [0x%x]\n"), szName, (long long)pVar);
    }
}

//---------------------------------------------------------------------------
void ListExprVar(mecParserHandle_t a_hParser)
{
    mecInt_t iNumVar = mecGetExprVarNum(a_hParser),
             i = 0;

    if (iNumVar == 0)
    {
        myprintf(_T("Expression dos not contain variables\n"));
        return;
    }

    myprintf(_T("\nExpression variables:\n"));
    myprintf(_T("---------------------\n"));
    myprintf(_T("Expression: %s\n"), mecGetExpr(a_hParser));
    myprintf(_T("Number: %d\n"), iNumVar);

    for (i = 0; i < iNumVar; ++i)
    {
        const mecChar_t* szName = 0;
        mecFloat_t* pVar = 0;

        mecGetExprVar(a_hParser, i, &szName, &pVar);
        myprintf(_T("Name: %s   Address: [0x%x]\n"), szName, (long long)pVar);
    }
}

//---------------------------------------------------------------------------
void ListConst(mecParserHandle_t a_hParser)
{
    mecInt_t iNumVar = mecGetConstNum(a_hParser),
        i = 0;

    if (iNumVar == 0)
    {
        myprintf(_T("No constants defined\n"));
        return;
    }

    myprintf(_T("\nParser constants:\n"));
    myprintf(_T("---------------------\n"));
    myprintf(_T("Number: %d"), iNumVar);

    for (i = 0; i < iNumVar; ++i)
    {
        const mecChar_t* szName = 0;
        mecFloat_t fVal = 0;

        mecGetConst(a_hParser, i, &szName, &fVal);
        myprintf(_T("  %s = %f\n"), szName, fVal);
    }
}

//---------------------------------------------------------------------------
/** \brief Check for external keywords.
*/
int CheckKeywords(const mecChar_t *a_szLine, mecParserHandle_t a_hParser)
{
    if (!mystrcmp(a_szLine, _T("quit")))
    {
        return -1;
    }
    else if (!mystrcmp(a_szLine, _T("list var")))
    {
        ListVar(a_hParser);
        return 1;
    }
    else if (!mystrcmp(a_szLine, _T("list exprvar")))
    {
        ListExprVar(a_hParser);
        return 1;
    }
    else if (!mystrcmp(a_szLine, _T("list const")))
    {
        ListConst(a_hParser);
        return 1;
    }
    else if (!mystrcmp(a_szLine, _T("locale de")))
    {
        myprintf(_T("Setting german locale: ArgSep=';' DecSep=',' ThousandsSep='.'\n"));
        mecSetArgSep(a_hParser, ';');
        mecSetDecSep(a_hParser, ',');
        mecSetThousandsSep(a_hParser, '.');
        return 1;
    }
    else if (!mystrcmp(a_szLine, _T("locale en")))
    {
        myprintf(_T("Setting english locale: ArgSep=',' DecSep='.' ThousandsSep=''\n"));
        mecSetArgSep(a_hParser, ',');
        mecSetDecSep(a_hParser, '.');
        mecSetThousandsSep(a_hParser, 0);
        return 1;
    }
    else if (!mystrcmp(a_szLine, _T("locale reset")))
    {
        myprintf(_T("Resetting locale\n"));
        mecResetLocale(a_hParser);
        return 1;
    }

    return 0;
}

//---------------------------------------------------------------------------
void Calc()
{
    mecChar_t szLine[100];
    mecFloat_t fVal = 0,
               afVarVal[] = { 1, 2, 7.2f, -2.1f }; // Values of the parser variables
    mecParserHandle_t hParser;
    mecEvalFun_t pFunEval = NULL;

    hParser = mecCreate();              // initialize the parser
    Intro(hParser);

    // Set an error handler [optional]
    // the only function that does not take a parser instance handle
    mecSetErrorHandler(hParser, OnError);

    //#define GERMAN_LOCALS
#ifdef GERMAN_LOCALS
    mecSetArgSep(hParser, ';');
    mecSetDecSep(hParser, ',');
    mecSetThousandsSep(hParser, '.');
#else
    mecSetArgSep(hParser, ',');
    mecSetDecSep(hParser, '.');
#endif

    // Define parser variables and bind them to C++ variables [optional]
    mecDefineConst(hParser, _T("const1"), 1);
    mecDefineConst(hParser, _T("const2"), 2);

    // Define parser variables and bind them to C++ variables [optional]
    mecDefineVar(hParser, _T("a"), &afVarVal[0]);
    mecDefineVar(hParser, _T("b"), &afVarVal[1]);
    mecDefineVar(hParser, _T("c"), &afVarVal[2]);
    mecDefineVar(hParser, _T("d"), &afVarVal[3]);

    // Define infix operator [optional]
    mecDefineInfixOprt(hParser, _T("!"), Not, 0);

    // Define functions [optional]
    mecDefineFun0(hParser, _T("zero"), ZeroArg, 0);
    mecDefineFun1(hParser, _T("rnd"), Rnd, 0);             // Add an unoptimizeable function
    mecDefineFun2(hParser, _T("dump"), DebugDump, 0);

    // Define binary operators [optional]
    mecDefineOprt(hParser, _T("add"), Add, 0, mecOPRT_ASCT_LEFT, 0);
    mecDefineOprt(hParser, _T("mul"), Mul, 1, mecOPRT_ASCT_LEFT, 0);

#ifdef _DEBUG
    mecDebugDump(1, 0);
#endif

    while (myfgets(szLine, 99, stdin))
    {
        szLine[mystrlen(szLine) - 1] = 0; // overwrite the newline

        switch (CheckKeywords(szLine, hParser))
        {
        case  0:  break;       // no keyword found; parse the line
        case  1:  continue;    // A Keyword was found do not parse the line
        case -1:  return;      // abort the application
        }

        // Set the expression
        mecSetExpr(hParser, szLine);


        // Compile the expression and get the pointer to the 
        // just in time compiled eval function
        pFunEval = mecDbgCompile(hParser, -1);
        if (pFunEval == NULL)
        {
            continue;
        }

        // calculate the expression
        fVal = pFunEval();

        /* alternative:
            fVal = mecEval(hParser); // 1st time parse from string and compile expression
            fVal = mecEval(hParser); // 2nd time parse from JIT code (handled internally)
            */
        // Without an Error handler function 
        // you must use this for error treatment:
        //if (mecError(hParser))
        //{
        //  printf("\nError:\n");
        //  printf("------\n");
        //  printf("Message:  %s\n", mecGetErrorMsg(hParser) );
        //  printf("Token:    %s\n", mecGetErrorToken(hParser) );
        //  printf("Position: %s\n", mecGetErrorPos(hParser) );
        //  printf("Errc:     %s\n", mecGetErrorCode(hParser) );
        //  continue;
        //}

        if (!mecError(hParser))
        {
            myprintf(_T("%f\n"), fVal);
        }

    } // while 

    // finalle free the parser ressources
    mecRelease(hParser);
}

//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    Calc();
    myprintf(_T("done..."));
}
