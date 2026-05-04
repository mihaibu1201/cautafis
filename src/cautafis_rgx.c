// cautafis_rgx.c

#define _DEFAULT_SOURCE

#include "../include/cautafis_rgx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <regex.h>

/*
 * Array global de structuri tip sEchivalRegex
 */
EcRgx pEcRgx[] = {
    {'*', ".*", 1},
    {'?', ".", 1},
    {'.', "\\.", 0}
};

/*
 * Variabilă globală folosită la regcomp o singură dată și apoi la regexec,
 * pentru fiecare nume de fișier, în timpul filtrării
 */
regex_t gRegex;
/*****************************************************************************/

/*****************************************************************************/
/* Funcția determină dacă argumentul pszNumeFis, reprezentând numele fișierului
 * primit argument la opțiunea -n la lansarea programului reprezintă nume
 * obișnuit de fișier, de ex fisier.txt, sau reprezinta o expresie glob, de
 * ex f?sier.t?t sau fis*.*.
 * Funcția parcurge argumentul pszNumeFis, ia fiecare dintre caracterele
 * componente, apoi acesta este verificat dacă apare în obiectul
 * în căutarea vreunui char din array-ul pEcRgx[], în câmpul m_cJoker.
 * Dacă cel puțin un caracter al pszNumeFis este găsit acolo, înseamnă că numele
 * fișierului este o expresie glob, iar acest lucru va determina modul de
 * filtrare: prin transformarea pszNumeFis într-o expresie Regex, iar filtrarea
 * se va face cu funcția nFiltreazaDenumireRegex.
 * Dacă nici un caracter nu este găsit în array, atunci pszNumeFis reprezintă
 * un nume simplu de fișier, iar filtrarea se face cu funcția nFiltreazaDenumire,
 * din cautafis.c
 * Apelat de: nParseazaLinieComanda() din cautafis.c
 * Parametri:
 *      - pszNumeFis: reprezintă numele fișierului, sau expresia glob care a
 *                      fost plasată programului la opțiunea -n
 * Return:
 *      - 1 dacă macar un caracter din pszNumeFis a fost găsit în array-ul global
 *          pEcRgx[],
 *      - 0, dacă nici un caracter din pszNumeFis nu a fost găsit în array-ul
 *          global pEcRgx[].
 */
int nVerificaExistJoker(const char* pszNumeFis)
{
int nRet;
int i;
    nRet = 0;
    i = 0;

    for(i = 0; i < NR_EXPR_REGEX_ECH; i++)
    {
        if (strchr(pszNumeFis, (unsigned int) pEcRgx[i].m_cJoker) && pEcRgx[i].m_bVerificare)
        {
            nRet = 1;
            break;
        }
    }

return nRet;
}//nVerificaExistJoker
/*****************************************************************************/

/*
 * Return:
 *      - 0, dacă compunerea expresiei Regex a fost ok;
 *      - 1, dacă compunerea expresiei Regex a ratat;
 *      - 2, dacă expresia Regex nu este compusă din caractere valide, sau
 *           sunt alte combinații eronate de caractere, paranteze, etc;
 *      - 3, dacă expresia Regex nu este validată Regex.
 */
/*
int nTransfNumeFisInExprRegex(const char* pszNumeFis, char* pszExprRegex)
{
int nRet;
int nRezEval;

    nRet = 0;
    nRezEval = 0;

    if(!nTransfExprCautareInRegex(pszNumeFis, pszExprRegex))
    {
        printf("Introdus: %s Obținut: %s; Lungime: %ld\n", pszNumeFis, pszExprRegex, strlen(pszExprRegex));

        nRezEval = nRegexValidareStricta(pszExprRegex);
        if(nRezEval == 1)
        {
            fprintf(stderr, "Eroare validare strictă caractere\n");
            nRet = 2;
        }
        if(!nRezEval)
        {
            nRezEval = nRegexValidareExpresie(pszExprRegex);
            if(nRezEval)
            {
                printf("Eroare validare expresie Regex\n");
                nRet = 3;
            }
            else
            {
                printf("Expresie Regex ok \n");
            }
        }
    }
    else
    {
        printf("Eroare transformare nume fișier în expresie Regex\n");
        nRet = 1;
    }

return nRet;
}//nVerificaFormatNumeFisier
*/
/*****************************************************************************/
/* Funcția transformă o expresie glob (pszGlob), într-o expresie validă regex
 * și o plasează în pszExpresieRegex.
 * Ex: pszGlob = d?mens*.* ; pszExpresieRegex va fi ^d.mens.*\..*$
 * Apelată de: nOptSetRegex() din cautafis.c.
 * Parametri:
 *      - pszGlob: expresia glob de transformat în expresie regex;
 *      - pszExpresieRegex: char* care va conține expresia regex finală.
 * Return:
 *      - 0: dacă transformarea s-a făcut cu succes,
 *      - 1: dacă transformarea a rata.
 */
int nTransfGlobInRegex(const char* pszGlob, char* pszExpresieRegex)
{
int nRet;
int i, j;
int nLungInput;
int nLungAct;
char* pszExprTmp;

char cCh;

    nRet = 0;
    i = j = nLungInput = nLungAct = 0;
    nLungInput = strlen(pszGlob);

    *pszExpresieRegex++ = '^';

    for(i = 0; i < nLungInput && !nRet ; i++)
    {
        cCh = (unsigned int) pszGlob[i];

        if((pszExprTmp = pszStrCoresp(cCh))!= (char*)NULL)
        {
            if(strlen(pszExpresieRegex + strlen(pszExprTmp)) >= MAX_LUNG_REGEX)
            {
                fprintf(stderr, "Expresie modificată prea lungă\n");
                nRet = 1;
                break;
            }
            while(*pszExprTmp)
            {
                *pszExpresieRegex++ = *pszExprTmp++;
            }
        }
        else
        {
            *pszExpresieRegex++ = cCh;
        }
    }
    *pszExpresieRegex++ = '$';
    *pszExpresieRegex = 0;


return nRet;
}//nTransfGlobInRegex
/*****************************************************************************/
/* Funcția primește argument un caracter, scanează array-ul global pEcRgx[] și
 * verifică dacă argumentul se află în vreo înregistrare în câmpul m_cJoker,
 * iar dacă există, întoarce câmpul m_pszExprEchRegex din înregistrarea
 * respectivă.
 * Apelată de: nTransfGlobInRegex().
 * Parametri:
 *      - cChJoker: caracterul care este verificat dacă există în câmpul
 *              m_cJoker din înregistrările din array-ul pEcRgx[].
 * Return:
 *      pszRet: char* la câmpul m_pszExprEchRegex, în caz că cChJoker există
 *              în câmpul m_cJoker în vreo înregistrare din array-ul pEcRgx[];
 *              (char*) NULL, dacă cChJoker nu există în câmpul m_cJoker în
 *              nici o înregistrare din array-ul pEcRgx[].
 */
char* pszStrCoresp(char cChJoker)
{
char* pszRet = {0};
size_t i;
int bGasit;

    bGasit = 0;
    i = 0;

    for(i = 0; i < sizeof(pEcRgx) / sizeof(pEcRgx[0]) && !bGasit; i++)
    {
        if(pEcRgx[i].m_cJoker == cChJoker)
        {
            pszRet = pEcRgx[i].m_pszExprEchRegex;
            bGasit = 1;
        }
    }

return pszRet;
}//pszStrCoresp
/*****************************************************************************/
/* Funcția verifică dacă argumentul cCh, care face parte dintr-o expresie regex,
 * este valabil.
 * Nu sunt admise:
 *   - | (alternation → crește complexitatea)
 *   - escape-uri dubioase (\b, \1, etc.)
 * Apelată de: nRegexValidareStricta().
 * Parametri:
 *      - cCh: caracterul de verificat.
 * Return:
 *      - 0: dacă cCh este valabil;
 *      - 1: dacă cCh nu este valabil.
 */
int nRegexCaracterAdmis(char cCh)
{
int nRet;

    nRet = 0;

    nRet = isalnum((unsigned char) cCh) ||
            cCh == '.' || cCh == '_' || cCh == '-' ||
            cCh == '^' || cCh == '$' || cCh == ' ' ||
            cCh == '*' || cCh == '+' || cCh == '?' ||
            cCh == '(' || cCh == ')' ||
            cCh == '[' || cCh == ']' ||
            cCh == '{' || cCh == '}' ||
            cCh == '\\'; // || cCh == '|';

return nRet;
}//nRegexCaracterAdmis
/*****************************************************************************/
/* Funcția verifică dacă argumentul pszExprRegex conține caracterele admise
 * pentru o expresie Regex, precum și dacă numărul de paranteze rotunde, drepte
 * și acolade este echilibrat. De asemenea, verifică dacă pszExprRegex depășește
 * lungimea maximă admisă a unei expresii regex, MAX_LUNG_REGEX.
 * Apelată de: nOptSetRegex() din cautafis.c
 * Parametri:
 *      - pszExprRegex: expresia regex de evaluat.
 * Return:
 *      - 0 dacă expresia este validă;
 *      - 1 dacă expresia nu este validă.
 */
int nRegexValidareStricta(const char* pszExprRegex)
{
int nRet;
size_t nLung;
int nBalantaParRot;
int nBalantaParDrept;
int nBalantaParAcola;
size_t i;
char cCh;
char cChPrev;

    nRet = 0;
    nLung = nBalantaParRot = nBalantaParDrept = nBalantaParAcola = i = 0;

    if (! *pszExprRegex)
    {
        // fprintf(stderr, "Pentru opțiunea -r, expresia Regex este obligatorie\n");
        fprintf(stderr, "Expresie Regex inexistentă\n");
        nRet = 1;
        goto fExit;
    }

    nLung = strlen(pszExprRegex);
    // printf("Lungime pezExprRegex: %ld\n", nLung);

    if(nLung == 0 || nLung > MAX_LUNG_REGEX)
    {
        fprintf(stderr, "Lungime necorespunzătoare a expresiei Regex: [%zd]\n", nLung);
        nRet = 1;
        goto fExit;
    }

    for (i = 0; i < nLung; i++)
    {
        cCh = pszExprRegex[i];
        // printf("În lucru: %c\n", cCh);

        // Doar caractere admise
        if (!nRegexCaracterAdmis(cCh))
        {
            printf("Caracter din expresie regex neadmis: %c\n", cCh);
            nRet = 1;
            break;
        }

        // Paranteze rotunde
        if (cCh == '(')
        {
            nBalantaParRot++;
            if (nBalantaParRot > MAX_NESTING_REGEX_ROT)
            {
                fprintf(stderr, "Depășire nivel maxim nexting regex paranteze rotunde: [%d]\n", nBalantaParRot);
                nRet = 1;
                break;
            }
        }

        // Paranteze drepte
        if (cCh == '[')
        {
            nBalantaParDrept++;
            if (nBalantaParDrept > MAX_NESTING_REGEX_DREPT)
            {
                fprintf(stderr, "Depășire nivel maxim nexting regex paranteze drepte: [%d]\n", nBalantaParDrept);
                nRet = 1;
                break;
            }
        }

        // Paranteze acolade
        if (cCh == '{')
        {
            nBalantaParAcola++;
            if (nBalantaParAcola > MAX_NESTING_REGEX_ACOLA)
            {
                fprintf(stderr, "Depășire nivel maxim nexting regex paranteze acolade: [%d]\n", nBalantaParAcola);
                nRet = 1;
                break;
            }
        }

        if (cCh == ')')
        {
            nBalantaParRot--;
            if (nBalantaParRot < 0)
            {
                fprintf(stderr, "Număr eronat de paranteze rotunde\n");
                nRet = 1;
                break;
            }
        }

        if (cCh == ']')
        {
            nBalantaParDrept--;
            if (nBalantaParDrept < 0)
            {
                fprintf(stderr, "Număr eronat de paranteze drepte\n");
                nRet = 1;
                break;
            }
        }

        if (cCh == '}')
        {
            nBalantaParAcola--;
            if (nBalantaParAcola < 0)
            {
                fprintf(stderr, "Număr eronat de paranteze acolade\n");
                nRet = 1;
                break;
            }
        }

        // Operatorii nu pot fi la început
        if (i == 0 && (cCh == '*' || cCh == '+' || cCh == '?'))
        {
            fprintf(stderr, "Operatorii nu pot fi la început\n");
            nRet = 1;
            break;
        }

        // Operatori consecutivi (ex: a**)
        if (i > 0)
        {
            cChPrev = pszExprRegex[i  - 1];
            if ((cCh == '*' || cCh == '+' || cCh == '?') &&
                (cChPrev == '*' || cChPrev == '+' || cChPrev == '?') &&
                !(cChPrev == '?' && cCh == '?'))
                {
                    fprintf(stderr, "Nu pot exista operatori consecutivi\n");
                    nRet = 1;
                    break;
                }
        }
    }

    if (nBalantaParRot != 0)
    {
        fprintf(stderr, "Număr eronat de paranteze rotunde pe ansamblu: %d\n", nBalantaParRot);
        nRet = 1;
    }

    if (nBalantaParDrept != 0)
    {
        fprintf(stderr, "Număr eronat de paranteze drepte pe ansamblu: %d\n", nBalantaParDrept);
        nRet = 1;
    }

    if(nBalantaParAcola != 0)
    {
        fprintf(stderr, "Număr eronat de paranteze acolade pe ansamblu: %d\n", nBalantaParAcola);
        nRet = 1;
    }

fExit:

return nRet;
}//nRegexValidareStricta
/*****************************************************************************/
/* Funcția verifică dacă argumentul pszExprRegex reprezintă o expresie regex
 * validă sau nu.
 * În caz că expresia nu este validă, funcția eliberează imediat resursele
 * alocate de regcomp(), cu regfree().
 * Apelată de: nOptSetRegex() din cautafis.c.
 * Parametri:
 *      - pOptCLI:        pointer la obiectul tip sOptiuni care conține opțiunile
 *                        pentru efectuarea căutarii și afișare;
 *      - pszExprRegex: expresia regex care trebuie verificată
 * Return:
 *      - 0: dacă expresia este validă;
 *      - 1: dacă expresia nu este validă.
 */
int nRegexValidareExpresie(OptCLI* pOpt, const char* pszExprRegex)
{
int nRet;
// Flag-uri folosite de regcomp
int nFlagComp;

    nRet = 0;
    nFlagComp = REG_EXTENDED | REG_NOSUB;
/* Dacă în linia de comandă a fost trimis parametrul -i, atunci căutarea se face
 * cu ignore-case, iar flag-ul pt regcomp este OR-at cu REG_ICASE
*/
    if(pOpt->m_nIgnoreCase == 1)
    {
        nFlagComp |= REG_ICASE;
    }

    nRet = regcomp(&gRegex, pszExprRegex, nFlagComp);
    if (nRet)
    {
        char errBuf[256];
        regerror(nRet, &gRegex, errBuf, sizeof(errBuf));
        fprintf(stderr, "Expresie regex invalidă: %s\n", errBuf);

        regfree(&gRegex);

    }

return nRet;
}//nRegexValidareExpresie
/*****************************************************************************/
/* Funcția realizeaza filtrarea propriu-zisă a denumirii fisierului pe baza
 * expresiei regex primită ca argument la lansarea programului, optiunea -r.
 * Apelată de: nFiltreazaFisier() din cautafis.c.
 * Parametri:
 *      - pszDenumireFis: numele fișierului care trebuie filtrat.
 * Return:
 *      - 1: dacă numele fișierului corespunde expresiei regex;
 *      - 0: dacă numele fișierului nu corespunde expresiei regex.
 */
int nFiltreazaDenumireRegex(const char* pszDenumireFis)
{
int nRet;
int nRezComp;

    nRet = 0;
    nRezComp = 0;

    if ((nRezComp = regexec(&gRegex, pszDenumireFis, 0, NULL, 0)) == 0)
    {
        nRet = 1;
    }

return nRet;
}//nFiltreazaDenumireRegex
/*****************************************************************************/
/* Funcția dezalocă variabila globală gRegex alocată de regcomp().
 * Apelată de: vDezalocaTot() din cautafis.c și nOptSetRegex() din
 *              cautafis.c.
 * Parametri: none.
 * Return: none
 */
void vDezalocaRegex()
{
    regfree(&gRegex);
}//vDezalocaRegex
/*****************************************************************************/
