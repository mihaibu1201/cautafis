/******************************************************************************
 * cautafis.c
 * Autor: Mihai Burcescu
 * Data: 04/05/2026
 * Ultima actualizare: 17/05/2026.
 * Programul caută într-un director trimis parametru, fișere și / sau directoare
 * care se aseamănă ca denumire cu numele trimis parametru, sau care corespund
 * cu o expresie regex. Se poate face o rafinare a căutării, în funcție de
 * opțunile trimise la lansarea programului.
 * Programul are opțiuni de rafinare a căutării:
 * -i sau --ignore-case
 * -f sau --fisiere
 * -d sau --directoare
 * -a sau --adancime <număr>
 * -m sau --min-size <număr>
 * -M sau --max-size <număr>
 * -s sau --sortare
 * -l sau --list-desc
 * -h sau --help
 * -v sau --versiune
 *
 * Opțiuni minime la start: cautafis -c <director start> -n <nume fișier>
 *                      sau cautafis -c <director start> -r <expresie regex>
 *                      sau cautafis -h
 *                      sau cautafis -v
 *
 * Pentru modul de folosire de către utilizator, detaliile sunt în fișierul
 * cautafis_hlp.txt.
 * Descrierea detaliată se află în README.md.
 *
 * Detalii tehnice: în fișierul tehnic.txt

 * Compilarea: vezi Makefile.
  */
#define _DEFAULT_SOURCE

#include "../include/cautafis.h"
#include <rgxfis.h>
#include "../include/versiune.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <getopt.h>
#include <ctype.h>
#include <time.h>


/*****************************************************************************/
int main(int argc, char** argv)
{
OptCLI *pOptCLI = {0};
CFis* pCFis = {0};
VLst *pVLst = {0};

int nOpt;
int nRez;
    nRez = 0;
    nOpt = 0;

/* Alocă resurse pentru obiect tip structura de opțiuni, pentru vectorul care
 * conține directoarele găsite și pentru vectorul care conține obiectele de
 * tip fișier și / sau director filtrate și adăugate pentru listare.
*/
    if (nAlocaTot(&pOptCLI, &pCFis, &pVLst) == ERR_ALOCARE)
        goto fExit;

// Verifică argumentele primite de program
    nOpt = nParseazaLinieComanda(pOptCLI, argc, argv);
    // printf("Rezultat nParseazaLinieComanda: %d\n", nOpt);
/*
 * Dunt 3 posibilități:
 * 1) Argumente ok, urmează executarea căutării și listării;
 * 2) Eroare argumente;
 * 3) Opțiunea de help - se afișează un text de help, din fișierul predefinit
 *      sau, dacă nu există, se afișează un text standard.
 */

    switch (nOpt){
        case OPT_OK:
            // vAfiseazaStructuraCLI(pOptCLI);
            nRez = nCautaFis(pOptCLI, pCFis, pVLst);
            // vListeazaVectorDirectoareDebug(pCFis);
            if (nRez == ERR_ALOCARE)
            {
                goto fExit;
            }
            else if (nRez == 0)
            {
                printf("Negăsit\n");
            }
            else if(nRez == 1)
            {
                vSorteazaVectorListare(pOptCLI, pVLst);
                if (nListeazaVectorListare(pVLst) == ERR_ALOCARE)
                    goto fExit;
            }

        break;
        case OPT_ERR:
            printf("Eroare parametri\n");
            vAfisHelpMinimal();
        break;
        case OPT_HELP:
            nRez = nAfisHelpFisier();
            if (!nRez)
            {
                vAfisHelpMinimal();
            }
            else if (nRez == ERR_ALOCARE)
            {
                goto fExit;
            }
        break;
        case OPT_VERS:
            vArataVersiunea();
        break;
        case ERR_ALOCARE:   // Aici doar dacă ratează la alocare în nOptSetRegex()
            goto fExit;
        break;

        default:
        break;
    }

fExit:

    vDezalocaTot(&pOptCLI, &pCFis, &pVLst);

return 0;
}//main
/*****************************************************************************/
/* Funcția alocă resurse pentru obiect tip structura de opțiuni, pentru vectorul
 * care conține directoarele găsite și pentru vectorul care conține obiectele de
 * tip fișier și / sau director filtrate și adăugate pentru listare.
 * Dacă oricare din funcțiile de alocare apelate ratează din cauza alocării,
 * funcția iese imediat în funcția main ()cu cod de eroare ERR_ALOCARE, iar
 * acolo este apelată direct vDezalocaTot() pentru dezalocarea tuturor resurselor
 * alocate pe heap, apoi ieșire.
 * Apelată de: main().
 * Apelează: nInitStructCLI(), nInitDirectoare, nInitListare().
 * Parametri:
 *      - pOptCLI:  pointer la obiectul tip sOptiuni care conține opțiunile
 *                  pentru efectuarea căutarii și afișare;
 *	    - pCFis:    pointer la obiectul tip sCautaFis ce conține vectorii
 *                  alocați pentru coada de directoare;
 *      - pVLst:    pointer la obiectul tip sVectorListare ce conține vectorii
 *                  alocați pentru listare.
 * Return:
 *      - 0, dacă alocarea s-a făcut corect;
 *      - ERR_ALOCARE, dacă alocarea nu s-a realizat.
 */
int nAlocaTot(OptCLI** pOptCLI, CFis** pCFis, VLst** pVLst)
{
int nRet;

    nRet = 0;

    *pOptCLI = NULL;
    *pCFis = NULL;
    *pVLst = NULL;

    if (nInitStructCLI(pOptCLI) == ERR_ALOCARE ||
            nInitListare(pVLst) == ERR_ALOCARE ||
            nInitDirectoare(pCFis) == ERR_ALOCARE)
        {
            nRet = ERR_ALOCARE;
            goto fExit;
        }

fExit:

return nRet;
}//nAlocaTot

/*****************************************************************************/
/* Funcția dezalocă toate resursele alocate pe heap.
 * În mod normal este apelată la finalul funcției main(), după ce se fac toate
 * căutările si listările, dacă e cazul.
 * Poate fi apelată dinăuntrul funcțiilor de alocare / realocare, caz în care
 * se afișează un mesaj de eroare, funcția se termină imediat, se ajunge în
 * main() și goto la fExit, punctul în care se apelează funcția vDezalocaTot().
 * Apelată de: main().
 * Apelează: vDezalocStructCLI(), vDezalocaDirectoare(), vDezalocListare().
 * Parametri:
 *      - pOptCLI: dublu pointer la obiectul tip sOptiuni care conține opțiunile
 *                 pentru efectuarea căutarii și afișare;
 *	    - pCFis:   dublu pointer la obiectul tip sCautaFis ce conține vectorii
 *                 alocați pentru coada de directoare;
 *      - pVLst:   dublu pointer la obiectul tip sVectorListare ce conține vectorii
 *                 alocați pentru listare.
 * Return: none
 * Sunt apelate funcțiile de dezalocare pentru fiecare argument.
*/
void vDezalocaTot(OptCLI** pOptCLI, CFis** pCFis, VLst** pVLst)
{
    vDezalocStructCLI(pOptCLI);
    vDezalocaDirectoare(pCFis);
    vDezalocListare(pVLst);
}//vDezalocaTot
/*****************************************************************************/
/* Funcția alocă un vector tip pointer la sOptiuni.
 * Apelată de: main()
 * Parametri:
 *      - pOptCLI:  dublu pointer la obiectul tip sOptiuni care conține opțiunile
 *                  pentru efectuarea căutarii și afișare.
 * Return:
 *      - 0, dacă alocarea s-a făcut corect;
 *      - ERR_ALOCARE, dacă alocarea nu s-a realizat.
 */
int nInitStructCLI(OptCLI ** pOptCLI)
{
int nRet;
    nRet = 0;

    *pOptCLI = (OptCLI*)malloc(sizeof(OptCLI));
    if(!(*pOptCLI))
    {
        fprintf(stderr, "Eroare alocare structură OptCli\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }

    memset(*pOptCLI, 0, sizeof(OptCLI));

fExit:

return nRet;
}//vInitStructCLI
/*****************************************************************************/
/* Funcția dezalocă vectorul aferent structurii sOptiuni.
 * Apelată de: vDezalocaTot().
 * Parametri:
 *      - pOptCLI:  dublu pointer la obiectul tip sOptiuni care conține opțiunile
 *                  pentru efectuarea căutarii și afișare.
 * Return: none
 */
void vDezalocStructCLI(OptCLI ** pOptCLI)
{
    if(*pOptCLI)
    {
        if((*pOptCLI)->m_pszDirector)
        {
            free ((*pOptCLI)->m_pszDirector);
            (*pOptCLI)->m_pszDirector = NULL;
        }

        if((*pOptCLI)->m_pszNumeFis)
        {
            free ((*pOptCLI)->m_pszNumeFis);
            (*pOptCLI)->m_pszNumeFis = NULL;
        }

        if((*pOptCLI)->m_pszExprRegex)
        {
            free ((*pOptCLI)->m_pszExprRegex);
            (*pOptCLI)->m_pszExprRegex = NULL;
        }

// Dacă s-a folosit expresie glob sau regex
        if((*pOptCLI)->m_gRegex.__allocated > 0)
        {
            vDezalocaRegex(&(*pOptCLI)->m_gRegex);
        }

        free(*pOptCLI);
        *pOptCLI = NULL;
    }


}//vDezalocStructCLI
/*****************************************************************************/
/* Afișează datele structurii tip sOptiuni
 * Parametri:
 *      - pOptCLI:  pointer la obiectul tip sOptiuni care conține opțiunile
 *                  pentru efectuarea căutarii și afișare;
 * Return: none
 */
void vAfiseazaStructuraCLI(OptCLI* pOptCLI)
{
    printf("Date structură OptCLI: \n");
    printf("Director bază: %s\n", pOptCLI->m_pszDirector);
    printf("Nume fișier: %s\n", pOptCLI->m_pszNumeFis);
    printf("Ignoră case: %d\n", pOptCLI->m_nIgnoreCase);
    printf("Tip toate/fișier/director: %d\n", pOptCLI->m_nTipFisier);
    printf("Adâncime: %d\n", pOptCLI->m_nAdancime);
    printf("Expresie Regex: %s\n", pOptCLI->m_pszExprRegex);
    printf("Dimensiune minimă: %ld\n", pOptCLI->m_lDimMin);
    printf("Dimensiune maximă: %ld\n", pOptCLI->m_lDimMax);
    printf("Tip sortare: %d\n", pOptCLI->m_nTipSortare);
    printf("Listare descendantă: %d\n", pOptCLI->m_nListareDesc);
    printf("----------------------------\n");

return;
}//vAfisStructura
/*****************************************************************************/
/* Afișează un help minimal.
 * Apelată de: main();
 * Parametri: none
 * Return: none
*/
void vAfisHelpMinimal()
{
    printf("Opțiuni program:\n");
    printf("----------------\n");
    printf("Opțiuni minime la lansarea programului:\n"
            "cautafis -c <director start> -n <nume fișier>\n"\
            "sau:\n"\
            "cautafis -c <director start> -r <expresie regex>\n"\
            "sau:\n"\
            "cautafis -h, caz în care se afișează un text help\n");
    printf("----------------\n");
    printf("Opțiuni de rafinare a căutării:\n" \
        "-i sau --ignore-case       -> nu se ține cont de litere mari și mici.\n"\
        "-f sau --fisiere           -> se caută doar fișiere.\n"\
        "-d sau --directoare        -> se caută doar directoare.\n"\
        "-a sau --adancime <număr>  -> numărul de subdirectoare în jos pt căutare.\n" \
        "-r sau --regex <expresie>  -> căutarea se face respectând regulile regex.\n"\
        "-m sau --min-size <număr>  -> sunt selectate doar fișierele cu dimensiunea\n"\
        "\t\t\t\tminimă de <număr> = MegaBytes.\n"\
        "-M sau --maxn-size <număr> -> sunt selectate doar fișierele cu dimensiunea\n"\
        "\t\t\t\tmaximă de <număr> = MegaBytes.\n"\
        "-s sau --sortare [a/s/d]   -> modul de sortare: \n"
        "\t\t\t\ta = după nume, s = după dimensiune, t = după data fișierului.\n"\
        "-l sau --list-desc         -> listarea se va face descendent;implicit este ascendent.\n"\
        "-h sau --help              -> se afișează un text help."\
        "-v sau --versiune          -> se afișează versiunea programului.");
    printf("\n");

return;
}//vAfisHelpMinimal
/*****************************************************************************/
/* Afișează un help prin copierea la ecran a conținutului fișierului de help
 * predefinit.
 * Apelată de: main();
 * Parametri: none.
  * Return:
 *      - 1, dacă fișierul a fost găsit;
 *      - 0, dacă fișierul nu a fost găsit;
 *      - ERR_ALOCARE, dacă alocarea de memorie pentru vectorul care urmează să
 *          conțină datele fișierului nu s-a realizat.
*/
int nAfisHelpFisier()
{
int nRet;
FILE *pFIn;
char* pszDataFis = {0};
size_t nDimData;
    nRet = 1;
    nDimData = 0L;

    // printf("Fișier help: %s\n", FISIER_HELP);

    if((pFIn = fopen(FISIER_HELP, "rt")) == NULL)
    {
        nRet = 0;   //fișier negăsit sau nu se deschide
    }
    else
    {
        if (fseek(pFIn, 0L, SEEK_END))
        {
            fclose(pFIn);
            nRet = 0;
        }
        else
        {
            nDimData = ftell(pFIn);
            if (fseek(pFIn, 0L, SEEK_SET))
            {
                nRet = 0;
            }
            else
            {
                if ((pszDataFis = malloc(nDimData)) == NULL)
                {
                    fprintf(stderr, "Eroare alocare memorie fișier help\n");
                    fclose(pFIn);
                    nRet = ERR_ALOCARE;
                    goto fExit;
                }
                else
                {
                    if(nDimData != fread(pszDataFis, sizeof(char), nDimData, pFIn))
                    {
                        printf("Eroare citire fișier help %s\n", FISIER_HELP);
                        fclose(pFIn);
                        nRet = 0;
                    }
                    else
                    {
                        printf("%s\n", pszDataFis);
                        fclose(pFIn);
                        free(pszDataFis);
                    }
                }
            }
        }
    }

fExit:

return nRet;
}//nAfisHelpFisier
/*****************************************************************************/
/*
 * Funcția parsează linia de comandă primită de program.
 * Apelată de:  main().
 * Apelează:    nOptDirector(), nOptNumeFis(), vOptSetCase(), vOptSetTipFisier(),
 *              nOptAdancime(), vOptSetRegex(), nOptCautDimensMinMax(),
 *              nOptTipSortare(), vOptSetListareDesc(), nValideazaOptiuni().
 * Parametri:
 *      - argc, argv sunt parametrii primiți de program la lansare și
 *          direcționați către această funcție.
 * Return:
 *      - 0, dacă sunt: 1) minim 5 parametri: numele programului, -d <director>,
 *                          -n <nume> și dacă toate opțiunile parametrilor
 *                          suplimentari au fost corecte;
 *                      2) 2 parametri, cu condiția ca al doilea să fie '-h'
 *                          sau '--help'.
 *      - 1, dacă nu s-au îndeplinit condițiile de mai sus;
 *      - 2, dacă a fost folosită opțiunea 'h', caz în care din main() se va
 *           apela o funcție care afișează un text help minimal.
 */
int nParseazaLinieComanda(OptCLI* pOpt, int argc, char ** argv)
{
int nRet;
int nOptiune;

int bHasDir;        // Opțiunea -c
int bHasNumeFis;    // Opțiunea -n
int bHasTipDir;     // Opțiunea -d
int bHasRegex;      // Optiunea -r
int bHasDimensMin;  // Opțiunea -m
int bHasDimensMax;  // Opțiunea -M
int bHasTipSortare; // Opțiunea -s

    nRet = OPT_OK;
    bHasDir = bHasNumeFis = bHasTipDir = bHasRegex = bHasDimensMin = bHasDimensMax = bHasTipSortare = 0;

struct option long_options[] = {
    {"cale", required_argument, 0,      'c'},
    {"nume", required_argument, 0,      'n'},
    {"ignore-case", no_argument, 0,     'i'},
    {"fisiere", no_argument, 0,         'f'},
    {"directoare", no_argument, 0,      'd'},
    {"adancime", required_argument, 0,  'a'},
    {"regex", required_argument, 0,     'r'},
    {"min-size", required_argument, 0,  'm'},
    {"max-size", required_argument, 0,  'M'},
    {"sortare", required_argument, 0,   's'},
    {"list-desc", required_argument, 0, 'l'},
    {"help", no_argument, 0,            'h'},
    {"versiune", no_argument, 0,        'v'},
    {0, 0, 0, 0}
};

    while ((nOptiune = getopt_long(argc, argv, "c:n:ifda:r:m:M:s:lhv", long_options, NULL)) != -1)
    {
        switch (nOptiune) {
            case 'h':
                nRet = OPT_HELP;
                goto fExit;
            break;
            case 'v':
                nRet = OPT_VERS;
                goto fExit;
            break;
            case 'c':
                bHasDir = 1;
                if (nOptSetDirector(pOpt, optarg))
                {
                    nRet = OPT_ERR;
                    goto fExit;
                }
            break;
            case 'n':
                bHasNumeFis = 1;
                if (nVerificaExistJoker(optarg))
                {
                    nRet = nOptSetRegex(pOpt, optarg, REGEX_INDIRECT);
                }
                else
                {
                    nRet = nOptSetNumeFis(pOpt, optarg);
                }
                if(nRet == 1)
                {
                    nRet = OPT_ERR;
                    goto fExit;
                }
            break;
            case 'i':
                vOptSetCase(pOpt);
            break;
            case 'f':
                vOptSetTipFisier(pOpt, TIPFISIER_FIS);
            break;
            case 'd':
                bHasTipDir = 1;
                vOptSetTipFisier(pOpt, TIPFISIER_DIR);
            break;
            case 'a':
                if(nOptSetAdancime(pOpt, optarg))
                {
                    nRet = OPT_ERR;
                    goto fExit;
                }
            break;
            case 'r':
                bHasRegex = 1;
                nRet = nOptSetRegex(pOpt, optarg, REGEX_DIRECT);
                if(nRet == OPT_ERR || nRet == ERR_ALOCARE)
                {
                    goto fExit;
                }
            break;
            case 'm':
                bHasDimensMin = 1;
                if(nOptSetDimensMinMax(pOpt, optarg, 0))
                {
                    nRet = OPT_ERR;
                    goto fExit;
                }
            break;
            case 'M':
                bHasDimensMax = 1;
                if(nOptSetDimensMinMax(pOpt, optarg, 1))
                {
                    nRet = OPT_ERR;
                    goto fExit;
                }
            break;
            case 's':
                if(nOptSetTipSortare(pOpt, optarg))
                {
                    nRet = OPT_ERR;
                    goto fExit;
                }
            break;
            case 'l':
                vOptSetListareDesc(pOpt);
            break;

            default:
            break;
        }
    }

    nRet = nValideazaOptiuni(bHasDir, bHasNumeFis, bHasTipDir, bHasRegex,
                             bHasDimensMin, bHasDimensMax, bHasTipSortare);

fExit:

return nRet;
}//nParseazaLinieComanda
/*****************************************************************************/
/* Funcția validează opțiunile primite de către program la lansare.
 * Nu valideaza aici parametrii opțiunilor, aceștia sunt validați de către
 * funcții ajutătoare.
 * Apelată de nParseazaLinieComanda().
 * Verifică existența obligatorie a opțiunilor -d și -n, sau doar a optiunii -h,
 * precum și a inexistenței simultane a opțiunilor -t și -s (director și dimensiune)
 * Parametri:
 *      -   bDir, bNume, bTip, bRegex, bDimensMin, bDimensMax, bTipSortare:
 *          dacă oricare dintre ele este egal cu 1, înseamnă că programul a primit
 *          opțiunea respectivă;
 * Return:
 *      OPT_OK, dacă toate condițiile de mai sus sunt îndeplinite;
 *      OPT_ERR, dacă nu sunt îndeplinite condițiile de mai sus;
 *      OPT_HELP, dacă a fost prezentă opțiunea -h
 */
int nValideazaOptiuni(int bDir, int bNume, int bTip, int bRegex,
                      int bDimensMin, int bDimensMax, int bTipSortare)
{
int nRet;
    nRet = OPT_OK;

    if (!bDir)
    {
        printf("Trebuie să fie prezent numele directorului de start\n");
        nRet = OPT_ERR;
    }
    else if (!bNume && !bRegex)
    {
        printf("Trebuie să fie prezent ori numele fișierului de căutat, (eventual cu wildcard-uri), "\
                    "ori o expresie regex cu numele de căutat\n");
        nRet = OPT_ERR;
    }
    else if (bNume && bRegex)
    {
        printf("Numele de fișier, (eventual cu wildcard-uri) și expresie regex cu numele de căutat "
            "nu pot fi prezente simultan\n");
        nRet = OPT_ERR;
    }
    else if ((bTip && bDimensMin) || (bTip && bDimensMax))
    {
        printf("Opțiunile de tip director și dimensiune nu pot fi prezente simultan.\n");
        nRet = OPT_ERR;
    }
    else if ((bTip && bTipSortare))
    {
        printf("Opțiunile de tip director și tip sortare nu pot fi prezente simultan;\n"\
        "Dacă sunt căutate doar directoare, acestea vor fi sortate după nume\n");
        nRet = OPT_ERR;
    }

return nRet;
}//nValideazaOptiuni
/*****************************************************************************/
/* Verifică existența parametrului director al opțiunii -c
 * Dacă există, alocă un vector de FILENAME_MAX și copiază în el șirul pszDir
 * primit argument. Asta deoarece argumentul primit de la utilizator ar putea
 * fi mai lung decât FILENAME_MAX, eventual posibil dDos.
 * Verifică dacă ultimul caracter al directorului este '/' și, dacă are această valoare,
 * îl elimină.
 * Dacă e ok, scrie câmpul m_pszDirector.
 * Apelată de nParseazaLinieComanda().
 * Funcția este fail-safe: dacă apare eroare de alocare memorie pe heap,funcția
 * iese în final în main() și memoria este eliberată apelând funcția vDezalocaTot().
 * Parametri:
 *      - pOptCLI: pointer la obiectul tip sOptiuni care conține opțiunile
 *                 pentru efectuarea căutarii și afișare;
 *      - pszDir:  pointer la un string reprezentând numele directorului.
 * Return:
 *      - OPT_OK, dacă pszDir există și are lungime > 0;
 *      - OPT_ERR, dacă nu există sau are lungime 0;
 *      - ERR_ALOCARE, dacă nu s-a putut aloca memorie pe heap pentru m_pszNumeDir.
 * Afișează mesaje de eroare dacă pszDir nu corespunde.
*/
int nOptSetDirector(OptCLI* pOptCLI, char* pszDir)
{
int nRet;
// Pt determinare dacă există '/' la final și, dacă e necesar, să-l scoată.
int nLung;

    nRet = OPT_OK;
    nLung = 0;

    if(pszDir == NULL || strlen(pszDir) <= 0)
    {
        printf("Pentru opțiunea -d, parametrul director este obligatoriu.\n");
        nRet = OPT_ERR;
        goto fExit;
    }

    pOptCLI->m_pszDirector = (char*)malloc(FILENAME_MAX);
    if(!pOptCLI->m_pszDirector)
    {
        fprintf(stderr, "Eroare alocare char* pOptCLI->m_pszDirector.\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }

// Copizază argumentul pszDir în câmpul m_pszDirector
    snprintf(pOptCLI->m_pszDirector, FILENAME_MAX, "%s", pszDir);

    //Elimină '/' de la final, dacă există;
    nLung = strlen(pOptCLI->m_pszDirector);
    if(*(pOptCLI->m_pszDirector + nLung - 1) == '/')
        *(pOptCLI->m_pszDirector + nLung -1) = 0;

fExit:

return nRet;
}//nOptSetDirector
/*****************************************************************************/
/* Verifică existența parametrului nume fișier al opțiunii -n
 * Dacă există, alocă un vector de FILENAME_MAX și copiază în el șirul pszNumeFis
 * primit argument. Asta deoarece argumentul primit de la utilizator ar putea
 * fi mai lung decât FILENAME_MAX, eventual posibil dDos.
 * Dacă e ok, scrie câmpul m_pszNumeFis.
 * Apelată de nParseazaLinieComanda().
 * Funcția este fail-safe: dacă apare eroare de alocare memorie pe heap,funcția
 * iese în final în main() și memoria este eliberată apelând funcția vDezalocaTot().
 * Parametri:
 *      - pOptCLI: pointer la obiectul tip sOptiuni care conține opțiunile
 *                 pentru efectuarea căutarii și afișare;
 *      - pszNumeFis:  pointer la un string reprezentând numele fișierului de căutat.
 * Return:
 *      - OPT_OK, dacă pszDir există și are lungime > 0;
 *      - OPT_ERR, dacă nu există sau are lungime 0;
 *      - ERR_ALOCARE, dacă nu s-a putut aloca memorie pe heap pentru m_pszNumeFis.
 * Afișează mesaje de eroare dacă pszNumeFis nu corespunde.
*/
int nOptSetNumeFis(OptCLI* pOptCLI, char* pszNumeFis)
{

int nRet;

    nRet = OPT_OK;

    if(pszNumeFis == NULL || strlen(pszNumeFis) <= 0)
    {
        printf("Pentru opțiunea -n, parametrul nume fișier este obligatoriu.\n");
        nRet = OPT_ERR;
        goto fExit;
    }

    if (nValidareNumeFis(pszNumeFis) != OPT_OK)
    {
        printf("Numele fișierului nu poate conține caracterul '/'.\n"
                "Caractere glob permise: '*' și '?'. Sau puteți folosi opțiunea -r\n");
        nRet = OPT_ERR;
        goto fExit;
    }

    pOptCLI->m_pszNumeFis = (char*)malloc(FILENAME_MAX);
    if(!pOptCLI->m_pszNumeFis)
    {
        fprintf(stderr, "Eroare alocare char* pOptCLI->m_pszNumeFis.\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }

// Copizază argumentul pszDir în câmpul m_pszDirector
    snprintf(pOptCLI->m_pszNumeFis, FILENAME_MAX, "%s", pszNumeFis);

fExit:

return nRet;
}//nOptSetNumeFis
/*****************************************************************************/
/* Funcția verifică existența caracterului '/' în numele fișierului.
 * Acest caracter nu poate exista pentru că se încurcă glob-ul :-D
 * Singurele caractere glob pot fi '*' și '?'.
 * Apelată de nOptSetNumeFis().
 * Parametri:
 *      - pszNumeFis: pointer la un string reprezentând numele fișierului de căutat.
 * Return:
 *      - OPT_OK, dacă argumentul pszNumeFis nu conține caracterul '/';
 *      - OPT_ERR, daca argumentul pszNumeFis conține caracterul '/'.
 */
int nValidareNumeFis(char* pszNumeFis)
{
int nRet;
    nRet = OPT_OK;

    if (strchr(pszNumeFis, '/') != NULL)
    {
        nRet = OPT_ERR;
    }

return nRet;
}//nValidareNumeFis
/*****************************************************************************/
/* Seteză valoarea m_nTipFisier pe 1 sau pe 2.
 * Apelată de nParseazaLinieComanda().
 * Parametri:
 *      - pOptCLI: pointer la obiectul tip sOptiuni care conține opțiunile
 *                 pentru efectuarea căutarii și afișare;
 *      - nTip:   TIPFISIER_FIS pentru fișier, TIPFISIER_DIR pentru director.
 * Return: none */
void vOptSetTipFisier(OptCLI* pOptCLI, int nTip)
{
    pOptCLI->m_nTipFisier = nTip;
}//vOptSetTipFisier
/*****************************************************************************/
/* Pentru opțiunea -a sau --adancime, parametrul reprezintă numărul de niveluri
 * de sub directorul de start în care se poate face căutarea.
 * Dacă e ok, scrie câmpul m_nAdancime.
 * Apelată de nParseazaLinieComanda().
 * Parametri:
 *      - pOptCLI:     pointer la obiectul tip sOptiuni care conține opțiunile
 *                     pentru efectuarea căutarii și afișare;
 *      - pszAdancime: string, parametrul opțiunii -a sau --adancime.
 * Return:
 *      0 dacă parametrul corespunde și s-a făcut corect strtol();
 *      1 la eroare, în caz că parametrul nu corespunde sau strtol() a ratat.
 * Afișează mesaje de eroare dacă pszAdancime nu corespunde.
 */
int nOptSetAdancime(OptCLI* pOptCLI, char* pszAdancime)
{
int nRet;
int nVal;
char *endptr;   // Pt strtol()
    nRet = OPT_OK;

    // nVal = atoi(pszAdancime);
    errno = 0;
    nVal = (int)strtol(pszAdancime, &endptr, 10);
    if (errno == ERANGE)
    {
        printf("Parametru necorespunzător: %s\n", pszAdancime);
        nRet = OPT_ERR;
    }
    else if(pszAdancime == endptr || *endptr != '\0')
    {
        printf("Parametru necorespunzător: %s\n", pszAdancime);
        nRet = OPT_ERR;
    }
    else if (nVal <= 0)
    {
        printf("Pentru opțiunea -adancime, parametrul trebuie să fie un număr pozitiv [%d].\n", nVal);
        nRet = OPT_ERR;
    }
    else
    {
        pOptCLI->m_nAdancime = nVal;
    }

return nRet;
}//nOptSetAdancime
/*****************************************************************************/
/* Setează valoarea câmpului m_lDimMin sau m_lDimMax.
 * Efectuează strtol(pszDimens).
 * Verifică valoarea din celălalt câmp și respectarea regulii
 * m_lDimMin < m_lDimMax.
 * Dacă e ok, scrie câmpul m_lDimMin, respectiv m_lDimMax, în funcție de
 * switch-ul nMinMax, în Bytes. Parametrul primit este în MegaBytes.
 * Valoarea primită în parametru (MegaBytes) se înmulțește cu 1048576 (1024 x 1024)
 * Apelată de nParseazaLinieComanda().
 * Parametri:
 *      - pOptCLI:   pointer la obiectul tip sOptiuni care conține opțiunile
 *                   pentru efectuarea căutarii și afișare;
 *      - pszDimens: string, reprezentând parametrul opțiunii -s (+/-)numar;
 *      - nMinMax:   switch:
 *          - dacă este zero, se ocupă de m_lDimMin;
 *          - dacă este unu, se ocupă de m_lDimMax.
 * Return:
 *      0 dacă parametrul corespunde și s-a făcut corect strtol();
 *      1 la eroare, în caz că parametrul nu corespunde sau strtol() a ratat,
 *      sau dacă nu se respectă regula m_lDimMin < m_lDimMax.
 * Afișează mesaje de eroare dacă pszDimens nu corespunde.
 */
int nOptSetDimensMinMax(OptCLI* pOptCLI, char* pszDimens, int nMinMax)
{
int nRet;
long lDim;

char *endptr;   // Pt strtol()
    nRet = OPT_OK;

    errno = 0;
    lDim = strtol(pszDimens, &endptr, 10);
    if (errno == ERANGE)
    {
        printf("Parametru necorespunzător: %s\n", pszDimens);
        nRet = OPT_ERR;
    }
    else if(pszDimens == endptr || *endptr != '\0')
    {
        printf("Parametru necorespunzător: %s\n", pszDimens);
        nRet = OPT_ERR;
    }
    else if(lDim <= 0)
    {
        printf("Pentru opțiunea mărime, parametrul trebuie să fie un număr pozitiv [%ld].\n", lDim);
        nRet = OPT_ERR;
    }
    else    //transformare ok :-)
    {
        if (nMinMax == 0)       //-s <=> minim
        {
            if(pOptCLI->m_lDimMax &&  (lDim >= pOptCLI->m_lDimMax))
            {
                printf("Dimensiunea minimă [input %ld] trebuie să fie strict "\
                "mai mică decât dimensiunea maximă [%ld].\n", lDim, pOptCLI->m_lDimMax);
                nRet = OPT_ERR;
            }
            else
            {
                pOptCLI->m_lDimMin = lDim  * 1048576;
            }
        }

        else if(nMinMax == 1)   //-S <=> maxim
        {
            if(pOptCLI->m_lDimMin && (lDim <= pOptCLI->m_lDimMin))
            {
                printf("Dimensiunea maximă [input %ld] trebuie să fie strict "\
                "mai mare decât dimensiunea minimă [%ld].\n", lDim, pOptCLI->m_lDimMin);
                nRet = OPT_ERR;
            }
            else
            {
                pOptCLI->m_lDimMax = lDim  * 1048576;
            }
        }
    }

return nRet;
}//nOptSetDimensMinMax
/*****************************************************************************/
/* Seteză valoarea m_nCase pe 1.
 * Apelată de nParseazaLinieComanda().
 * Parametri:
 *      - pOptCLI: pointer la obiectul tip sOptiuni care conține opțiunile
 *                 pentru efectuarea căutarii și afișare.
 * Return: none
*/
void vOptSetCase(OptCLI* pOptCLI)
{
    pOptCLI->m_nIgnoreCase = 1;
}//vOptSetCase
/*****************************************************************************/
/* Verifică existența parametrului Expresie Regex al opțiunii -r.
 * Face validarea expresiei, care include, preventiv, anumite reguli verificate
 * de funcțiile de validare regex definite în program.
 * Dacă e ok, scrie câmpul m_pszExprRegex.
 * Apelată de nParseazaLinieComanda().
 * Funcția este fail-safe: dacă apare eroare de alocare memorie pe heap,funcția
 * iese în final în main() și memoria este eliberată apelând funcția vDezalocaTot().
 * Parametri:
 *      - pOptCLI: pointer la obiectul tip sOptiuni care conține opțiunile
 *                 pentru efectuarea căutarii și afișare;
 *      - pszInput: poate exprima două posibilități:
 *                      1) expresia Regex introdusă la lansarea programului;
 *                      2) expresia Nume Fișier introdusă la lansarea programului,
 *                          dar care conține jokere.
 *      - nTipArgument:
 *              - REGEX_DIRECT, dacă a existat argumentul -r în linia de comandă;
 *              - REGEX_INDIRECT, dacă a existat argumentul -n din linia de comandă,
 *                  dar numele de fișier conține jokere => devine expresie Regex.
 *
 * Return:
 *      - OPT_OK, dacă expresia există și este validă Regex;
 *      - OPT_ERR,  dacă expresia nu există sau nu este validă Regex;
 *      - ERR_ALOCARE, dacă nu s-a putut aloca memorie pe heap pentru m_pszExprRegex.
*/
int nOptSetRegex(OptCLI* pOptCLI, char* pszInput, int nTipArgument)
{
int nRet;
char *pszExprRegex;
    nRet = OPT_OK;

    pszExprRegex = (char*)malloc(MAX_LUNG_REGEX);
    if(!pszExprRegex)
    {
        fprintf(stderr, "Eroare alocare char* pszExprRegex.\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }

    if(nTipArgument == REGEX_INDIRECT)
    {
        nRet = nTransfGlobInRegex(pszInput, pszExprRegex);
        // printf("Indirect: de la %s la %s\n", pszInput, pszExprRegex);
        if (strlen(pszExprRegex) > MAX_LUNG_REGEX)
        {
            printf("Lungime expresie Regex prea mare\n");
            nRet = 1;
        }

        if (nRet != 0)
        {
            nRet = OPT_ERR;
            goto fExit;
        }
    }
    else
    {
        snprintf(pszExprRegex, MAX_LUNG_REGEX, "%s", pszInput);
    }

    nRet = nRegexValidareStricta(pszExprRegex);
    if (nRet == 0)
    {
        nRet = nRegexValidareExpresie(&pOptCLI->m_gRegex, pOptCLI->m_nIgnoreCase, pszExprRegex);
    }
    else
    {
        nRet = OPT_ERR;
        goto fExit;
    }

    if (nRet == 0)
    {
        pOptCLI->m_pszExprRegex = pszExprRegex;
    }
    else
    {
        nRet = OPT_ERR;
        goto fExit;
    }

fExit:
    if (nRet != 0)
    {
        free (pszExprRegex);

        if(pOptCLI->m_gRegex.__allocated > 0)
        {
            vDezalocaRegex(&pOptCLI->m_gRegex);
        }
    }

return nRet;
}//nOptSetRegex
/*****************************************************************************/
/* Pentru opțiunea -s sau --sortare, parametrul reprezintă tipul de sortare
 * al fișierelor și / sau directoarelor care respectă criteriul de filtrare.
 * Dacă e ok, scrie câmpul m_nTipSortare.
 * Apelată de nParseazaLinieComanda().
 *  * Parametrul din pszTipSortare poate fi:
 *      -'a', pentru sortare Ascii, a numelui fișierului;
 *      -'s', pentru sortare în funcție de dimensiunea fișierului;
 *      -'d', pentru sortare în funcție de data ultimei modificări a fișierului.
 * Parametri:
 *      - pOptCLI:       pointer la obiectul tip sOptiuni care conține opțiunile
 *                       pentru efectuarea căutarii și afișare;
 *      - pszTipSortare: string, parametrul opțiunii -s sau --sortare.
 * Return:
 *      0 dacă parametrul corespunde; asta presupune ca pszTipSortare[0] să
 *          conțină doar 'a' sau 's' sau 'd'.
 *      1 la eroare, în caz că parametrul nu corespunde.
 * Afișează mesaje de eroare dacă pszTipSortare nu corespunde.
 * Notă: Vizualizarea finală se face și în funcție de tipul de sortare, dar și
 * în funcție de modul de vizualizare (ascendent sau descendent), dat de prezența
 * opțiunii --list-desc, setat prin funcția nSetListareDesc.
 */
int nOptSetTipSortare(OptCLI* pOptCLI, char* pszTipSortare)
{
int nRet;
char cCh;

    nRet = OPT_OK;

    cCh = *pszTipSortare;
    if(cCh == 'a')      // Sortare Ascii, după numele fișierului
    {
        pOptCLI->m_nTipSortare = TIPSORTARE_NUME;
    }
    else if(cCh == 's') // Sortare numerica, după dimensiunea fișierului
    {
        pOptCLI->m_nTipSortare = TIPSORTARE_DIMENSIUNE;
    }
    else if(cCh == 'd') // Sortare numerica, după data ultimei modificări a fișierului.
    {
        pOptCLI->m_nTipSortare = TIPSORTARE_DATA;
    }
    else
    {
        printf("Parametrul pentru opțiunea --sortare poate fi 'a' sau 's' sau 'd'.\n");
        nRet = OPT_ERR;
    }

return nRet;
}//nOptSetTipSortare
/*****************************************************************************/
/* Seteză valoarea m_nListareDesc pe 1.
 * Apelată de nParseazaLinieComanda().
 * Parametri:
 *      - pOptCLI: pointer la obiectul tip sOptiuni care conține opțiunile
 *                 pentru efectuarea căutarii și afișare.
 * Return: none
*/
void vOptSetListareDesc(OptCLI* pOptCLI)
{
    pOptCLI->m_nListareDesc = 1;
}//nSetListareDesc
/******************************************************************************/
/* Funcția alocă un vector tip CFis și vectorul component m_pDirCaut, cu
 * dimensiunea egală cu 1 x sizeof(struct sDirector).
 * Apelată de: main().
 * Parametri:
 *	    - pCFis: dublu pointer la obiectul tip sCautaFis ce conține vectorii
 *               alocați pentru coada de directoare.
 * Return:
 *      0, dacă alocarea s-a făcut cu succes;
 *      ERR_ALOCARE, dacă alocarea a ratat.
 * În caz că alocarea a ratat, funcția iese din punctul în care a ratat,
 * se ajunge în main și de acolo se apelează vDezalocaTot()
 */
int nInitDirectoare(CFis ** pCFis)
{
int nRet;
    nRet = 0;

    *pCFis = (CFis*)malloc(sizeof(CFis));
    if(pCFis == NULL)
    {
        fprintf(stderr, "Eroare alocare pCFis\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }
    else
    {
        memset(*pCFis, 0, sizeof(CFis));
        (*pCFis)->m_nNrAlocate = 1;
        (*pCFis)->m_pDirCaut = malloc((*pCFis)->m_nNrAlocate * sizeof((*pCFis)->m_pDirCaut));
        if(!(*pCFis)->m_pDirCaut)
        {
            fprintf(stderr, "Eroare alocare m_pDirCaut\n");
            nRet = ERR_ALOCARE;
            goto fExit;
        }
    }

fExit:

return nRet;
}//nInitDirectoare
/******************************************************************************/
/* Funcția eliberează memoria ocupată de vectorul tip CFis și vectorul component
 * m_pDirCaut, cu dimensiunea m_nNrDirGasite x sizeof(struct sDirector), precum
 * și a vectorilor m_pszNumeDir din cadrul acestora.
 * Funcția verifică dacă s-a făcut în prealabil dezalocarea acestora, pentru a
 * nu face dezalocare dublă.
 * Apelată de: vDezalocaTot().
 * Parametri:
 *	    - pCFis: dublu pointer la obiectul tip sCautaFis ce conține vectorii
 *               alocați pentru coada de directoare.
 * Return: none
*/
void vDezalocaDirectoare(CFis** pCFis)
{
int i;
    i = 0;

    if (*pCFis)
    {
        for (i = 0; i < (*pCFis)->m_nNrDirGasite; i++)
        {
            if ((*pCFis)->m_pDirCaut[i]->m_pszNumeDir)
            {
                free ((*pCFis)->m_pDirCaut[i]->m_pszNumeDir);
                (*pCFis)->m_pDirCaut[i]->m_pszNumeDir = 0;
            }

            if ((*pCFis)->m_pDirCaut[i])
            {
                free ((*pCFis)->m_pDirCaut[i]);
                (*pCFis)->m_pDirCaut[i] = 0;
            }
        }

        if ((*pCFis)->m_pDirCaut)
        {
            free ((*pCFis)->m_pDirCaut);
            (*pCFis)->m_pDirCaut = 0;
        }

        free (*pCFis);
        *pCFis = 0;
    }

}//vDezalocaDirectoare
/*****************************************************************************
 * nCautaFis: funcția deschide directorul primit argument pszDir, străbate
 * acest director, filtrează numele fiecărui obiect de tip fișier și / sau
 * subdirector și,în caz că numele corespunde, îl adaugă într-un vector care
 * conține numele ce vor fi listate.
 * Dacă în director există un subdirector, acesta este adăugat într-o coadă,
 * sunt analizate toate fișierele din acest director, apoi, după terminare,
 * este adus următorul director din coadă, apoi parcurge aceiași pași în acel
 * director.
 * Ori de câte ori este găsit un subdirector al (sub)directorului aflat în
 * lucru, acesta este ađăugat într-o coadă de directoare și este incrementată
 * valoarea membrului pCFis->m_nNrDirGasite.
 * Analizarea tuturor fișierelor dintr-un director, se face în secvența dintre
 * opendir() și closedir(), cu readdir().
 * Când este terminată scanarea unui director, cu funcția readdir(), este
 * incrementată variabila locală nDirCrt. Aceasta conține numărul de directoare
 * scanate complet.
 * La un moment dat se termină directoarele scanate, iar nDirCrt va ajunge egal
 * cu pCFis->m_nNrDirGasite - 1. În acel moment, scanarea tuturor fișierelor din
 * directorul de start și al subdirectoarelor acestuia este încheiată.
 * În timpul scanării fiecărui obiect (fișier și / sau subdirector) este făcută
 * filtrarea acestuia (comparația cu numele fișierului primit argument de către
 * program).
 * De asemenea, este determinat dacă fișierul în curs este un fișier normal sau
 * director. Dacă este director, este determinată și adâncimea acestuia, adică
 * diferența de arborescență față de directorul de start.
 * Dacă între argumentele primite de program a fost prezentă și -a (adâncime
 * maximă), funcția nu va permite ca scanarea să continue în subdirectoarele care
 * depășesc adâncimea primită parametru.
 * Dacă un obiect este filtrat, atunci el este adăugat în vectorul de listare,
 * urmând ca funcția main() să continue cu sortarea și listarea acestuia.
 * Funcția este fail-safe: dacă apar erori, indiferent de locul în care acestea
 * pot apare, funcția iese în main() și memoria este eliberată apelând funcția
 * vDezalocaTot().
 * Parametrii primiți reprezintă pointeri la structuri care au fost alocate în
 * funcția main().
 * Apelată de: main().
 * Apeleaza: nAdaugaDirector(), nFiltreazaFisier(), nAdaugareFisier().
 * Parametri:
 *      - pOptCLI:  pointer la obiectul tip sOptiuni care conține opțiunile
 *                  pentru efectuarea căutarii și afișare;
 *	    - pCFis:    pointer la obiectul tip sCautaFis ce conține vectorii
 *                  alocați pentru coada de directoare;
 *      - pVLst:    pointer la obiectul tip sVectorListare ce conține vectorii
 *                  alocați pentru listare.
 * Return:
 *      - 1 dacă a găsit vreun fișier;
 *      - 0 dacă nu a găsit niciun fișier;
 *      - ERR_ALOCARE la eroare de alocare, din oricare din funcții.
*/
int nCautaFis(OptCLI* pOptCLI, CFis* pCFis, VLst* pVLst)
{
int nRet;
DIR *dp;
// pszDir = directorul de start, preluat din structura de opțiuni CLI
char *pszDir;
// pszNume = numele fișierului de căutat, preluat din structura de opțiuni CLI
struct dirent *ep;
struct stat statbuf;
// Pentru compunerea căii complete în timpul scanării unui subdirector.
char* pszCale;
// Numărul directorului care se scanează în mod curent.
size_t nDirCrt;
// Dacă fișierul în lucru este sau nu director.
int bEDirector;
// Dacă fișierul în lucru este admis în vectorul de fișiere și / sau directoare pt
// listarea finală. Accesul este dat de funcția nFiltreazaAcces()
int nAccesFiltrare;
// Nivelul în arborescență al directorului curent
int nNivelCrt;
/* Dacă e sau nu nevoie de stat() pt filtrare - doar când e nevoie de filtrare
 * după dimensiune.
*/
int bTrebStat;

/* Variabila bStatFacut este folosită pentru optimizarea apelării funcției
 * stat(). Aceasta este o funcție foarte costisitoare.
 *
 * Funcția nCautaFis scanează arborescența de directoare de sub directorul
 * primit argument în linia de comandă. Pentru aceasta, are nevoie să determine
 * dacă obiectul scanat cu readdir() este sau nu de tip DT_DIR. Sunt două
 * posibilități pentru a determina acest lucru:
 *  - consultând variabila d_type a structurii tip stat folosită de readdir();
 *  - apelând funcția stat().
 * Deci, prima posibilă apelare a funcției stat(), poate fi evitată prin
 * consultarea câmpului d_type.
 * Dacă d_type este == DT_UNKNOWN, atunci se apelează stat(), iar variabila
 * locală bStatFacut este setat pe unu.
 *
 * A doua oară când este posibil să se apeleze stat() este înaintea filtrării.
 * Filtrarea se face asupra numelui în mod obligatoriu, dar și asupra dimensiunii,
 * dacă a fost folosită opțiunea --min-size și / sau --max-size la lansarea
 * programului. În acest ultim caz, este nevoie de dimensiunea fișierului,
 * iar singura funcție care întoarce această valoare este stat().
 * Această a două apelare a funcției stat() se face dacă bTrebStat este egal cu
 * 1. Variabila bTrebStat verifică dacă este nevoie de apel pentru dimensiune
 * și dacă a fost sau nu apelată mai sus, adică dacă bStatFacut == 1 sau nu.
 * Dacă stat() a fost făcut, variabila bStatFacut este deja pe valoarea 1,
 * iar stat() nu se mai face.
 *
 * A treia oară când este posibil să se apeleze stat() este înaintea plasării
 * obiectului filtrat în vectorul de obiecte de listat.
 * Pentru listare este nevoie de valorile dimensiunii (dacă obiectul este
 * strict de tip TIPFISIER_FIS), precum și de valoarea datei ultimei modificări.
 * Dacă variabila bStatFacut este setat pe 1, atunci stat() nu este apelată
 * în acest ultim caz.
 *
 * Variabila bStatFacut permite să se apeleze o singură dată funcția stat(),
 * în loc de potențial trei ori, aceasta fiind o optimizare importantă în cadrul
 * programului.
 */
int bStatFacut;

/* Pointer la o funcție de comparație stringuri, care va depinde de furnizarea
 * sau nu a opțiunii -i la lansarea programului. Pointerul va fi trimis către
 * funcția nFiltreazaFisier(), care, la rândul ei, îl va trimite la funcția
 * nFiltreazaDenumire().
 */
char* (*pFiltrare)(const char*, const char*);

    nRet = 0;
    nDirCrt = 0;
    bEDirector = 0;
    nAccesFiltrare = 0;
    nNivelCrt = -1;
    bTrebStat = 0;
    bStatFacut = 0;


    pszDir = pOptCLI->m_pszDirector;
    assert(pszDir);

// Atribuirea pointerului către funcția de comparare stringuri.
    if (pOptCLI->m_nIgnoreCase)
        pFiltrare = strcasestr;
    else
        pFiltrare = strstr;


    pszCale = (char*)malloc(FILENAME_MAX);
    if(!pszCale)
    {
        fprintf(stderr, "Eroare alocare pszCale\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }

// Se adaugă directorul de start, primit argument de către program la lansare.
    if (nAdaugaDirector(pCFis, pszDir, nNivelCrt) == ERR_ALOCARE)
        goto fExit;

    while(nDirCrt < (size_t)pCFis->m_nNrDirGasite)
    {
        dp = opendir(pCFis->m_pDirCaut[nDirCrt]->m_pszNumeDir);
        // printf("Lucrez în: %s\n", pCFis->m_pszVectDirs[nDirCrt]);
        // Nu se poate deschide directorul respectiv, se renunță la analiză.
        if (!dp)
        {
            // printf("Director șters: %s\n", pCFis->m_pszVectDirs[nDirCrt]);
            nDirCrt++;
            continue;
        }
        else
        {
            /* Nivelul directorului curent - este determinat prin accesarea
             * membrului m_nNivel din cadrul structurii tip DirCaut al
             * directorului aflat în lucru; va fi incrementat în subdirectoarele
             * acestuia și va fi salvat în variabila lor m_nNivel.
            */
            nNivelCrt = pCFis->m_pDirCaut[nDirCrt]->m_nNivel;

            // Scanare director curent
            while ((ep = readdir(dp))!= NULL)
            {
                bStatFacut = 0;
                if(strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "..") != 0)
                {

                    snprintf(pszCale, FILENAME_MAX, "%s/%s",  pCFis->m_pDirCaut[nDirCrt]->m_pszNumeDir, ep->d_name);
                    //memset(&statbuf, 0, sizeof(statbuf));

                    /* Aici încerc să evit apelul stat(), dacă e posibil. Dacă ep->d_type == DT_UNKNOWN,
                     * trebuie apelat stat, apoi, în funcție de rezultatul stat() și al S_ISDIR, variabila
                     * bEDirector ia valoarea 1 sau 0.
                     * Altfel, dacă ep->d_type == DT_DIR (4), atunci bEDirector ia valoarea 1, altfel
                     * ia valoarea 0.
                     * Soluția este ok local, pentru că va mai fi nevoie de stat() și înaintea filtrării,
                     * de asemenea și în timpul adăugarii obiectului în vectorul obiecte pentru listare.
                     * Mai multe explicații sunt la declararea variabilei bStatFacut.
                     */
                    if (ep->d_type == DT_DIR)    // DT_DIR = 4
                    {
                        bEDirector = 1;
                    }
                    else if(ep->d_type == DT_REG) //DT_REG = 8
                    {
                        bEDirector = 0;
                    }
                    else if (ep->d_type == DT_LNK)  //DT_LNK = 10
                    {
                        continue;
                    }
                    else if (ep->d_type == DT_UNKNOWN) //DT_UNKNOWN = 0
                    {
                        if (-1 == stat(pszCale, &statbuf))
                        {
                            fprintf(stderr, "Eroare stat: %d; cale: %s\n", errno, pszCale);
                            continue;
                        }

                        bStatFacut = 1;

                        if (S_ISLNK(statbuf.st_mode))
                            continue;

                        bEDirector = S_ISDIR(statbuf.st_mode);

                    }
                    else    //alte tipuri: fifo, socket, etc
                    {
                        bEDirector = 0;
                    }

                    //if(S_ISDIR(statbuf.st_mode))
                    //if(ep->d_type == 4)
                    if (bEDirector)
                    {
                        //printf("Nr director curent: %d\n", pCFis->m_nNr);
                        // Dacă depășește adâncimea din CLI, nu continuă cu adăugarea
                        if(pOptCLI->m_nAdancime && nNivelCrt >= pOptCLI->m_nAdancime)
                            continue;
                        if (nAdaugaDirector(pCFis, pszCale, nNivelCrt) == ERR_ALOCARE)
                            goto fExit;
                    }

/*
 * Aici se apelează funcțiile de filtrare
 */
                    // printf("Final buclă cale: %s\n", pszCale);
                    /* Fișierul nu este de tip DT_UNKNOWN și nu este director, caz
                     * în care nu s-a folosit stat(), ci tipul de fișier pentru
                     * instucțiunile de mai sus a fost determinat pe baza
                     * valorii din câmpul ep->d_type.
                     * Având nevoie de câmpul stat.st_size, trebuie să apelăm
                     * stat(). (1)
                     * Dacă fișierul este de tip DT_UNKNOWN și nu este director,
                     * atunci putem folosi valorile din structura statbuf, care
                     * a fost deja alocată de apelul stat().
                     * Aici suntem în situația (1).
                    */
                    /* Dacă s-a făcut deja stat() pentru acest obiect, nu se mai face
                     * încă o dată și se folosesc datele din variabila statbuf pentru
                     * nFiltreazaFisier - este o optimizare importantă, pt că stat()
                     * este costisitoare.
                     */

                    /* stat() nefăcut și există cerințele de filtrare după dimensiune.
                     * Se face stat() acum, iar în continuare, pentru filtrare, se vor
                     * folosi datele din variabila statbuf inițializate la acest apel
                     * al stat().
                     */
                    bTrebStat = !bStatFacut && (ep->d_type == DT_UNKNOWN ||
                                    pOptCLI->m_lDimMin || pOptCLI->m_lDimMax);

                    if (bTrebStat)
                    {
                        if (-1 == stat(pszCale, &statbuf))
                        {
                            // fprintf(stderr, "Eroare stat la filtrare: %d; cale: %s\n", errno, pszCale);
                            continue;
                            //nAccesFiltrare = 0;
                        }
                        bStatFacut = 1;
                    }

                    /* Obiectul tip fișier și / director este analizat dacă 'seamănă' cu parametrul
                     * primit în argumentul -n de către program.
                     * Dacă el corespunde, este adăugat în vectorul de listare.
                     */
                    //nAccesFiltrare = nFiltreazaFisier(pOptCLI, &statbuf, pszCale, bEDirector);
                    // Filtrarea se face strict asupra numelui fișierului care se află în lucru.
                    nAccesFiltrare = nFiltreazaFisier(pOptCLI, &statbuf, ep->d_name, bEDirector, pFiltrare);
                    if(nAccesFiltrare)
                    {
                        //printf("Fișier găsit: %s\n", pszCale);

                        /* A (posibil) treia apelare a funcției stat(), de data
                         * asta doar pentru listare, pentru a se plasa valorile
                         * st_size pt dimensiune st_mtim pt data ultimei modificări.
                         * Această a (posibil) treia apelare se face în funcție
                         * de valoarea variabilei bStatFacut.
                         */
                        if(!bStatFacut)
                        {
                            if (-1 == stat(pszCale, &statbuf))
                            {
                                // fprintf(stderr, "Eroare stat la filtrare: %d; cale: %s\n", errno, pszCale);
                                continue;
                                //nAccesFiltrare = 0;
                            }
                            bStatFacut = 1;
                        }
                        if (nAdaugareFisier(pVLst, &statbuf, pszCale, bEDirector) == ERR_ALOCARE)
                        {
                            nRet = ERR_ALOCARE;
                            goto fExit;
                        }
                        // Fișier adăugat cu succes
                        nRet = 1;
                    }
                }
            }

            (void)closedir(dp);
            // printf("Director șters: %s\n", pCFis->m_pszVectDirs[nDirCrt]);
            nDirCrt++;
        }
    }

    free(pszCale);
    pszCale = 0;

fExit:

return nRet;
}//nCautaFis

/*****************************************************************************/
/* Funcția adaugă un director nou în coada de directoare 'descoperite'.
 * Verifică dacă memoria alocată pentru vectorul m_pDirCaut, în cantitate de
 * m_nNrAlocate * sizeof(m_pDirCaut) mai este disponibilă. Dacă s-a terminat,
 * atunci câmpul m_nNrAlocate se dublează și se face realloc pt vectorul
 * m_pDirCaut, folosind inițial un vector temporar.
 * Apoi se alocă memorie pentru câmpul m_pszNumeDir și se copiază în el
 * cu snprintf() numele directorului pszNumeDir primit argument.
 * Se incrementează numărul nivelului nNivelParinte, pe care îl are directorul
 * părinte, și se depoziteaza valoarea incrementată în câmpul m_nNivel.
 * Se incrementează câmpul m_nNrDirGasite.
 * Dacă la realloc() sau malloc() apar erori, funcția iese imediat cu cod de
 * eroare ERR_ALOCARE, se ajunge în main() unde sunt eliberate toate resursele
 * cu vDezalocaTot().
 * Apelată de: nCautaFis().
 * Parametri:
 *	    - pCFis: pointer la obiectul tip sCautaFis ce conține vectorii alocați
 *               pentru coada de directoare;
 *      - pszNumeDir: numele directorului de adăugat;
 *      - nNivelParinte: nivelul directorului părinte al directorului pszNumeDir.
 * Return:
 *      0, dacă nu sunt erori de alocare sau copiere;
 *      ERR_ALOCARE, dacă sunt erori de aloare.
 */
int nAdaugaDirector(CFis* pCFis, char* pszNumeDir, int nNivelParinte)
{
int nRet;
DirCaut** pTmp;

    nRet = 0;

    if(pCFis->m_nNrAlocate <= pCFis->m_nNrDirGasite)
    {
        pCFis->m_nNrAlocate *= 2;
        pTmp = realloc(pCFis->m_pDirCaut, pCFis->m_nNrAlocate * sizeof(pCFis->m_pDirCaut));
        if(!pTmp)
        {
            fprintf(stderr, "Eroare alocare pentru m_pDirCaut\n");
            nRet = ERR_ALOCARE;
            goto fExit;
        }
        pCFis->m_pDirCaut = pTmp;
    }

    pCFis->m_pDirCaut[pCFis->m_nNrDirGasite] = malloc(sizeof(DirCaut));
    if (!pCFis->m_pDirCaut[pCFis->m_nNrDirGasite])
    {
        fprintf(stderr, "Eroare alocare pCFis->m_pDirCaut[i]\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }

    pCFis->m_pDirCaut[pCFis->m_nNrDirGasite]->m_pszNumeDir = (char*)malloc(FILENAME_MAX);
    if (!pCFis->m_pDirCaut[pCFis->m_nNrDirGasite]->m_pszNumeDir)
    {
        fprintf(stderr, "Eroare alocare char* pentru copiere nume director\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }
// Copiere nume director primit argument în pszNumeDir
    snprintf(pCFis->m_pDirCaut[pCFis->m_nNrDirGasite]->m_pszNumeDir, FILENAME_MAX, "%s", pszNumeDir);
// Incrementare numărul nivelului directorului părinte primit în argumentul nNivelParinte și
//     salvarea acestuia în m_nNivel.
    pCFis->m_pDirCaut[pCFis->m_nNrDirGasite]->m_nNivel = nNivelParinte + 1;
// Incrementare număr de directoare găsite.
    pCFis->m_nNrDirGasite++;

fExit:

return nRet;
}//nAdaugaDirector
/*****************************************************************************/
/* Funcția listează toate directoarele găsite și nivelul lor din arborescență,
 * pornind de la directorul bază, care are nivel zero.
 * Apelată de: main().
 * Parametri:
 *	    - pCFis: pointer la obiectul tip sCautaFis ce conține vectorii alocați
 *               pentru coada de directoare.
 * Return: none
 * Notă: funcția este folosită doar pt debug.
 */
void vListeazaVectorDirectoareDebug(CFis* pCFis)
{
int i;

    for (i = 0; i < pCFis->m_nNrDirGasite; i++)
    {
        printf("Director: %s\tNivel: %d\n", pCFis->m_pDirCaut[i]->m_pszNumeDir,
               pCFis->m_pDirCaut[i]->m_nNivel);
    }

}//vListeazaVectorDirectoareDebug

/*****************************************************************************/
/* Funcția compară stringurile argument pszDenumire cu pszModel.
 * În funcție de setarea din sOptiuniCLI pentru m_nIgnoreCase, primită în linia
 * de comandă a programului, comparația se poate face utilizând strcasestr(),
 * sau strstr().
 * Dacă opțiunea m_nIgnoreCase din opțiuni este pe 0, atunci nu se ignoră case
 * și se folosește strcasestr(), altfel se folosește strstr().
 * Apelată de funcția de filtrare principală nFiltreazaFisier().
 * Parametri:
 *      - pOptCLI:        pointer la obiectul tip sOptiuni care conține opțiunile
 *          pentru efectuarea căutarii și afișare;
 *      - pszDenumireFis: numele fișierului, sau directorului care se compară cu
 *          stringul din câmpul m_pszNumeFis din sOptiuniCLI), primit în linia
 *          de comandă;
 *      - pFiltrare:      pointer la funcția de comparare stringuri, care poate
 *          fi strcasestr() sau strst(), în funcție de prezența opțiunii -i la
 *          lansarea programului.
 * Return:
 *      1, dacă pszDenumire 'seamănă' cu pszModel;
 *      0, dacă cele două stringuri n-au nici o treabă unu cu altu :-D
 */
int nFiltreazaDenumire(OptCLI *pOptCLI, const char* pszDenumireFis, char*(*pFiltrare)(const char*, const char*))
{
int nRet;
const char* pszModel;

    nRet = 0;
    pszModel = (const char*) pOptCLI->m_pszNumeFis;

    if ((*pFiltrare)(pszDenumireFis, pszModel))
        nRet = 1;

return nRet;
}//nFiltreazaDenumire
/*****************************************************************************/
/* Funcția stabilește dacă fișierul sau directorul respectiv poate face parte
 * din selecție. Este prima funcție de filtrare din șirul de funcții de filtrare
 * apelate de funcția de filtrare principală nFiltreazaFisier().
 * În funcție de argumentul bEDirector și de opțiunea m_nTipFisier din structura
 * de tip sOptiuni, fișierul respectiv poate face parte sau nu din selecție.
 * Sunt verificate tipul de acces din sOpțiuni și tipul de fișier curent
 * (normal sau director).
 * Valorile din opțunea din sOptiuni m_nTipFisier:
 *      - TIPFISIER_TOT pt toate, TIPFISIER_FIS pentru fișier,
 *        TIPFISIER_DIR pentru director.
 * Apelată de funcția de filtrare principală nFiltreazaFisier().
 * Parametri:
 *      - pOptCLI:    pointer la obiectul tip sOptiuni care conține opțiunile
 *                    pentru efectuarea căutarii și afișare;
 *      - bEDirector: switch care indică ce tip de fișier este analizat:
 *          0: fișier normal, 1 director.
 * Return:
 *      1, dacă combinația dintre bEdirector și m_nTipFisier 'trece';
 *      0, daca combinația 'nu trece'.
 */
int nFiltreazaTipFisier(OptCLI* pOptCLI, int bEDirector)
{
int nRet;

    nRet = 0;
    // Fișier obișnuit
    if (!bEDirector)
    {
        nRet = ((pOptCLI->m_nTipFisier == 0) || (pOptCLI->m_nTipFisier == 1));
    }
    else if(bEDirector)     // Director
    {
        nRet = ((pOptCLI->m_nTipFisier == 0) || (pOptCLI->m_nTipFisier == 2));
    }

return nRet;
}//nFiltreazaTipFisier
/*****************************************************************************/
/* Funcția stabilește daca fișierul sau directorul de analizat poate face parte
 * din selecție, în funcție de dimensiune.
 * Dacă este director, nu îi ese permis accesul din această funcție, pentru că
 * dacă a fost invocat --min-size sau --max-size, atunci nu este vorba despre
 * directoare.
 * Verifică dacă valorile din câmpurile m_lDimMin și m_lDimMax sunt > 0, iar în
 * funcție de valorile lor face comparație cu dimensiunea fișierului, dată de
 * câmpul (statbuf->st_size / 1024), pentru a se obține valoarea în MegaBytes,
 * așa cum sunt și valorile din câmpurile m_lDimMin și m_lDimMin.
 * Apelată de către funcția de filtrare principală nFiltreazaFisier().
 * Parametri:
 *      - pOptCLI:    pointer la obiectul tip sOptiuni care conține opțiunile
 *                    pentru efectuarea căutarii și afișare;
 *      - pStatBuf:   pointer la o structură tip stat care conține date despre
 *                    fișierul analizat;
 *      - bEDirector: switch care indică ce tip de fișier este analizat:
 *          0: fișier normal, 1 director.
 * Return:
 *      1,  dacă bEDirector == 1, deci fișierul de analizat este un director,
 *          sau dacă dimensiunea din pStatBuf->st_size se încadrează între
 *          valorile din pOpt->m_lDimMin și pOpt->m_lDimMax, în funcție de
 *          cum sunt setate;
 *      0,  dacă dimensiunea fișierului nu respectă cerințele de mai sus.
*/
int nFiltreazaDimensiune(OptCLI* pOptCLI, struct stat* pStatBuf, int bEDirector)
{
int nRet;
// Dimensiunea fișierului de analizat, in MegaBytes.
long lDimFis;

    nRet = 0;
//     Dacă e director, nu îi este permis accesul dacă se face căutare după dimensiune.
    if (bEDirector && (pOptCLI->m_lDimMin || pOptCLI->m_lDimMax))
    {
        nRet = 0;
    }
    else    //fișier normal, este filtrat în funcție de dimensiune.
    {
        lDimFis = (long)(pStatBuf->st_size);

        // Nesetate nici --min-size nici --min-size
        if (!pOptCLI->m_lDimMin && !pOptCLI->m_lDimMax)
        {
            nRet = 1;
        }
        // Setate și --min-size și --max-size
        else if (pOptCLI->m_lDimMin && pOptCLI->m_lDimMax)
        {
            nRet = lDimFis >= pOptCLI->m_lDimMin && lDimFis <= pOptCLI->m_lDimMax;
        }
        else if(pOptCLI->m_lDimMin)    // Setată doar --min-size
        {
            nRet = lDimFis >= pOptCLI->m_lDimMin;
        }
        else if(pOptCLI->m_lDimMax)    // Setată doar --max-size
        {
            nRet = lDimFis <= pOptCLI->m_lDimMax;
        }
    }

return nRet;
}//nFiltreazaDimensiune
/*****************************************************************************/
/* Funcția efectuează filtrarea unui fișier în vederea plasării acestuia în
 * vectorul char* reprezentănd fișierele și / sau directoarele acceptate pentru
 * listingul final.
 * Filtrarea se face în urma comparării caracteristicilor fișierului de filtrat
 * cu opțiunile din obiectul tip sOptiuniCLI, în care sunt plasate argumentele
 * și parametrii din linia de comandă a programului.
 * Apelată de: nCautaFis();
 * Apelează funcțiile de filtrare:
 *  - nFiltreazaTipFisier, pentru filtrarea tipului de fișier sau director,
 *  - nFiltreazaDimensiune, pentru filtrarea în funcție de dimensiune (doar
 *      dacă este fișier normal; pentru director, este permis din acestă funcție),
 *  - nFiltreazaDenumire, pentru filtrare în funcție de denumire; se poate face
 *      cu ignorarea Case sau cu respectarea Case.
 * Parametri:
 *      - pOptCLI:        pointer la obiectul tip sOptiuni care conține opțiunile
 *                        pentru efectuarea căutarii și afișare;
 *      - pStatBuf:       pointer la o structură tip stat care conține date despre
 *                        fișierul analizat;
 *      - pszDenumireFis: numele fișierului, sau directorului care se compară
 *                        cu stringul din câmpul m_pszNumeFis din sOptiuniCLI),
 *                        primit în linia de comandă.
 *      - bEDirector:     switch care indică ce tip de fișier este analizat:
 *          0: fișier normal, 1 director.
 *      - pFiltrare:      pointer la funcția de comparare stringuri, care poate
 *                        fi strcasestr() sau strst(), în funcție de prezența
 *                        opțiunii -i la lansarea programului.
 * Return:
 *      1, dacă toate funcțiile apelate au întors 1;
 *      0, dacă oricare dintre funcțiile apelate a întors 0.
 *
 * Rezultatul 1 asigură accesul fișierului respectiv în vectorul de char*
 * reprezentând fișierele care fac parte din listingul final.
 */
int nFiltreazaFisier(OptCLI* pOptCLI, struct stat * pStatBuf, char *pszDenumireFis, int bEDirector,
                     char*(*pFiltrare)(const char*, const char*))
{
int nRet;
    nRet = 1;

    if(pOptCLI->m_pszNumeFis == NULL && pOptCLI->m_pszExprRegex == NULL)
    {
        fprintf(stderr, "Eroare filtrare: nu există nici nume fișier, nici expresie Regex\n");
        nRet = 0;
    }

    nRet = nRet && nFiltreazaTipFisier(pOptCLI, bEDirector) &&
            nFiltreazaDimensiune(pOptCLI, pStatBuf, bEDirector);

    if (pOptCLI->m_pszNumeFis != NULL)
    {
        nRet &= nFiltreazaDenumire(pOptCLI, pszDenumireFis, pFiltrare);
    }
    else if (pOptCLI->m_pszExprRegex != NULL)
    {
        nRet &= nFiltreazaDenumireRegex(&pOptCLI->m_gRegex, pszDenumireFis);
    }

return nRet;
}//nFiltreazaFisier
/*****************************************************************************/
/* Funcția alocă un vector tip VLst și vectorul component m_pLstFis, cu
 * dimensiunea egală cu 1 x sizeof(struct sListareFis).
 * Apelată de: main().
 * Parametri:
 *      - pVLst: dublu pointer la obiectul tip sVectorListare ce conține vectorii
 *              alocați pentru listare.
 * Return:
 *      0, dacă alocarea s-a făcut cu succes;
 *      ERR_ALOCARE, dacă alocarea a ratat.
 * În caz că alocarea a ratat, funcția iese din punctul în care a ratat,
 * se ajunge în main și de acolo se apelează vDezalocaTot()
 */

/* Partea de listare */

/*****************************************************************************/
int nInitListare(VLst ** pVLst)
{
int nRet;
    nRet = 0;

    *pVLst = (VLst*)malloc(sizeof(VLst));
    if(pVLst == NULL)
    {
        fprintf(stderr, "Eroare alocare pVLst\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }
    else
    {
        memset(*pVLst, 0, sizeof(VLst));
        (*pVLst)->m_nNrAlocate = 1;
        (*pVLst)->m_pLstFis = malloc((*pVLst)->m_nNrAlocate * sizeof((*pVLst)->m_pLstFis));
        if(!(*pVLst)->m_pLstFis)
        {
            fprintf(stderr, "Eroare alocare m_pLstFis\n");
            nRet = ERR_ALOCARE;
            goto fExit;
        }
    }

fExit:

return nRet;
}//nInitListare
/*****************************************************************************/
/* Funcția eliberează memoria ocupată de vectorul tip VLst și vectorul component
 * m_pLstFis, cu dimensiunea m_nNrObAdaugate x sizeof(struct sListareFis), precum
 * și a vectorilor m_pszNumeFis din cadrul acestora.
 * Funcția verifică dacă s-a făcut în prealabil dezalocarea acestora, pentru a
 * nu face dezalocare dublă.
 * Apelată de vDezalocaTot().
 * Parametri:
 *      - pVLst: dublu pointer la obiectul tip sVectorListare ce conține vectorii
 *               alocați pentru listare.
 * Return: none
*/
void vDezalocListare(VLst ** pVLst)
{
int i;
    i = 0;

    if(*pVLst)
    {
        for(i = 0; i < (*pVLst)->m_nNrObAdaugate; i++)
        {
            if ((*pVLst)->m_pLstFis[i]->m_pszNumeFis)
            {
                free ((*pVLst)->m_pLstFis[i]->m_pszNumeFis);
                (*pVLst)->m_pLstFis[i]->m_pszNumeFis = 0;
            }

            if((*pVLst)->m_pLstFis[i])
            {
                free ((*pVLst)->m_pLstFis[i]);
                (*pVLst)->m_pLstFis[i] = 0;
            }
        }

        if ((*pVLst)->m_pLstFis)
        {
            free ((*pVLst)->m_pLstFis);
            (*pVLst)->m_pLstFis = 0;
        }

        free (*pVLst);
        *pVLst = 0;
    }

}// vDezalocListare
/*****************************************************************************/
/* Funcția adaugă un nou obiect de tip fișier / sau director în vectorul de
 * obiecte pentru listare.
 * Verifică dacă memoria alocată pentru vectorul m_pLstFis, în cantitate de
 * m_nNrAlocate * sizeof(m_pLstFis) mai este disponibilă. Dacă s-a terminat,
 * atunci câmpul m_nNrAlocate se dublează și se face realloc pt vectorul
 * m_pLstFis, folosind inițial un vector temporar.
 * Apoi se alocă memorie pentru câmpul m_pszNumeFis și se copiază în el
 * cu snprintf() numele directorului pszNumeFis primit argument.
 * Câmpul m_nTipFisier va avea valoarea 1, dacă argumentul bEDirector este
 * zero, sau 0, dacă bEDirector este 1.
 * Câmpurile m_lDimFis și m_tDataFis sunt completațe cu valorile st_size și,
 * respectiv st_mtim->tv_sec ai argumentului statbuf, doar dacă valoarea
 * argumentului bStatFacut este egală cu 1.
 * Notă: dacă obiectul este de tip director, atunci se copiază numai
 * câmpurile m_pszNumeFis, m_nTipFisier și m_tDataFis. Câmpul m_lDimFis nu
 * este completat, fiind vorba de un director.
 * Se incrementează câmpul m_nNrObAdaugate.
 * Dacă la realloc() sau malloc() apar erori, funcția iese imediat cu cod de
 * eroare ERR_ALOCARE, se ajunge în main() unde sunt eliberate toate resursele
 * cu vDezalocaTot().
 * Apelată de: nCautaFis().
 * Parametri:
 *      - pVLst:      pointer la obiectul tip sVectorListare ce conține vectorii
 *                    alocați pentru listare;
 *      - statbuf:    pointer la structura tip stat determinată prin invocarea
 *                    funcției stat asupra fișierului primit argument.
 *      - pszNumeFis: numele obiectului de tip fișier și /s sau director de adăugat;
 *      - bEDirector: switch,
 *          - 0 dacă obiectul nu e tip director;
 *          - 1 dacă obiectul e tip director;
 * Return:
 *      0, dacă nu sunt erori de alocare sau copiere;
 *      ERR_ALOCARE, dacă sunt erori de aloare.
 */
int nAdaugareFisier(VLst* pVLst, struct stat* statbuf, char* pszNumeFis, int bEDirector)
{
int nRet;
LstFis** pTmp;

    nRet = 0;

    if(pVLst->m_nNrAlocate <= pVLst->m_nNrObAdaugate)
    {
        pVLst->m_nNrAlocate *= 2;
        pTmp = realloc(pVLst->m_pLstFis, pVLst->m_nNrAlocate * sizeof(pVLst->m_pLstFis));
        if (!pTmp)
        {
            fprintf(stderr, "Eroare realocare pentru m_pLstFis\n");
            nRet = ERR_ALOCARE;
            goto fExit;
        }

        pVLst->m_pLstFis = pTmp;
    }

    pVLst->m_pLstFis[pVLst->m_nNrObAdaugate] = malloc(sizeof(LstFis));
    if(!pVLst->m_pLstFis[pVLst->m_nNrObAdaugate])
    {
        fprintf(stderr, "Eroare alocare pVLst->m_pLstFis[i]\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }

    pVLst->m_pLstFis[pVLst->m_nNrObAdaugate]->m_pszNumeFis = (char*)malloc(FILENAME_MAX);
    if(!pVLst->m_pLstFis[pVLst->m_nNrObAdaugate]->m_pszNumeFis)
    {
        fprintf(stderr, "Eroare alocare char* pentru copiere nume fișier\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }

// Copiere nume fișier primit argument în pszNumeFis.
    snprintf(pVLst->m_pLstFis[pVLst->m_nNrObAdaugate]->m_pszNumeFis, FILENAME_MAX, "%s", pszNumeFis);
// Determinare tip fișier și plasare în m_nTipFisier.
    pVLst->m_pLstFis[pVLst->m_nNrObAdaugate]->m_nTipFisier = bEDirector == 1 ? TIPFISIER_DIR: TIPFISIER_FIS;

// Determinare m_lDimFis din argumentul statbuf, doar dacă obiectul nu este de tip TIPFISIER_DIR
    if (pVLst->m_pLstFis[pVLst->m_nNrObAdaugate]->m_nTipFisier != TIPFISIER_DIR)
        pVLst->m_pLstFis[pVLst->m_nNrObAdaugate]->m_lDimFis = statbuf->st_size;
// Determinare m_tDataFis pe baza valorilor din argumentul statbuf.
    pVLst->m_pLstFis[pVLst->m_nNrObAdaugate]->m_tDataFis = statbuf->st_mtim.tv_sec;

    // Incrementare nr de obiecte adăugate.
    pVLst->m_nNrObAdaugate++;

fExit:

return nRet;
}//nAdaugareFisier
/*****************************************************************************/
/* Funcție ajutătoare, este apelată de qsort - funcția 'compare'.
 * Face comparația între numele a două elemente tip LstFis consecutive, apelând
 * funcția strcmp(), pentru câmpurile m_pszNumeFis.
 * Este folosită pentru sortarea ascendentă, se face scaderea între primul și
 * cel de-al doilea element.
 * Apelată de: nSorteazaVectorListare().
 * Parametri:
 *      - d1, d2 pointeri const la void*, sunt cast la LstFis*
 * Return:
 *      -   < 0, dacă primul element este mai mic decât al doilea element;
 *      -     0, dacă primul și cel de-al doilea element sunt egale;
 *      -   > 0, al doilea element este mai mare decât primul element.
 */
int nListSortNumeAsc(const void* d1, const void* d2)
{
const LstFis *p1, *p2;

    p1 = *(const LstFis**)d1;
    p2 = *(const LstFis**)d2;

    return (strncmp(p1->m_pszNumeFis, p2->m_pszNumeFis, FILENAME_MAX));
}//nListSortNumeAsc
/*****************************************************************************/
/* Funcție ajutătoare, este apelată de qsort - funcția 'compare'.
 * Face comparația între dimensiunile a două elemente tip LstFis consecutive.
 * Este folosită pentru sortarea ascendentă, se face scaderea dintre primul și
 * cel de-al doilea element, pentru câmpurile m_lDimFis.
 * Apelată de nSorteazaVectorListare().
 * Parametri:
 *      - d1, d2 pointeri const la void*, sunt cast la LstFis*
 * Return:
 *      -   < 0, dacă primul element este mai mic decât al doilea element;
 *      -     0, dacă primul și cel de-al doilea element sunt egale;
 *      -   > 0, al doilea element este mai mare decât primul element.
 */
int nListSortDimAsc(const void* d1, const void* d2)
{
const LstFis *p1, *p2;

    p1 = *(const LstFis **)d1;
    p2 = *(const LstFis**)d2;
    return (p1->m_lDimFis - p2->m_lDimFis);
}//nListSortDimAsc
/*****************************************************************************/
/* Funcție ajutătoare, este apelată de qsort - funcția 'compare'.
 * Face comparația între dimensiunile a două elemente tip LstFis consecutive.
 * Este folosită pentru sortarea ascendentă, se face scaderea dintre primul și
 * cel de-al doilea element, pentru câmpurile m_tDataFis.
 * Apelată de nSorteazaVectorListare().
 * Parametri:
 *      - d1, d2 pointeri const la void*, sunt cast la LstFis*
 * Return:
 *      -   < 0, dacă primul element este mai mic decât al doilea element;
 *      -     0, dacă primul și cel de-al doilea element sunt egale;
 *      -   > 0, al doilea element este mai mare decât primul element.
 */
int nListSortDataAsc(const void* d1, const void* d2)
{
const LstFis *p1, *p2;

    p1 = *(const LstFis**)d1;
    p2 = *(const LstFis**)d2;

    return (p1->m_tDataFis - p2->m_tDataFis);
}//nListSortDataAsc
/*****************************************************************************/
/* Funcție ajutătoare, este apelată de qsort - funcția 'compare'.
 * Face comparația între numele a două elemente tip LstFis consecutive, apelând
 * funcția strcmp(), pentru câmpurile m_pszNumeFis.
 * Este folosită pentru sortarea descendentă, se face scaderea între cel de-al
 * doilea element și primul element.
 * Apelată de nSorteazaVectorListare().
 * Parametri:
 *      - d1, d2 pointeri const la void*, sunt cast la LstFis*
 * Return:
 *      -   < 0, dacă primul element este mai mic decât al doilea element;
 *      -     0, dacă primul și cel de-al doilea element sunt egale;
 *      -   > 0, al doilea element este mai mare decât primul element.
 */
int nListSortNumeDesc(const void* d1, const void* d2)
{
const LstFis *p1, *p2;

    p1 = *(const LstFis**)d1;
    p2 = *(const LstFis**)d2;

    return (strncmp(p2->m_pszNumeFis, p1->m_pszNumeFis, FILENAME_MAX));
}//nListSortNumeDesc
/*****************************************************************************/
/* Funcție ajutătoare, este apelată de qsort - funcția 'compare'.
 * Face comparația între dimensiunile a două elemente tip LstFis consecutive.
 * Este folosită pentru sortarea descendentă, se face scaderea dintre cel de-al
 * doilea element și primul element, pentru câmpurile m_lDimFis.
 * Apelată de nSorteazaVectorListare().
 * Parametri:
 *      - d1, d2 pointeri const la void*, sunt cast la LstFis*
 * Return:
 *      -   < 0, dacă primul element este mai mic decât al doilea element;
 *      -     0, dacă primul și cel de-al doilea element sunt egale;
 *      -   > 0, al doilea element este mai mare decât primul element.
 */
int nListSortDimDesc(const void* d1, const void* d2)
{
const LstFis *p1, *p2;

    p1 = *(const LstFis**)d1;
    p2 = *(const LstFis**)d2;

    return (p2->m_lDimFis - p1->m_lDimFis);
}//nListSortDimDesc
/*****************************************************************************/
/* Funcție ajutătoare, este apelată de qsort - funcția 'compare'.
 * Face comparația între dimensiunile a două elemente tip LstFis consecutive.
 * Este folosită pentru sortarea descendentă, se face scaderea dintre cel de-al
 * doilea element și primul element, pentru câmpurile m_tDataFis.
 * Apelată de nSorteazaVectorListare().
 * Parametri:
 *      - d1, d2 pointeri const la void*, sunt cast la LstFis*
 * Return:
 *      -   < 0, dacă primul element este mai mic decât al doilea element;
 *      -     0, dacă primul și cel de-al doilea element sunt egale;
 *      -   > 0, al doilea element este mai mare decât primul element.
 */
int nListSortDataDesc(const void* d1, const void* d2)
{
const LstFis *p1, *p2;

    p1 = *(const LstFis**)d1;
    p2 = *(const LstFis**)d2;

    return (p2->m_tDataFis - p1->m_tDataFis);
}//nListSortDataDesc
/*****************************************************************************/
/* Funcția sortează vectorul care conține obiectele de tip fișier și / sau
 * director care au fost filtrate de funcția de căutare și vor fi listate.
 * Tipul de sortare este determinat de valoarea câmpului m_nTipSortare din
 * structura de tip sOptiuni, în care se află valorile argumentelor și
 * parametrilor trimiși în linia de comandă a programului.
 * De asemenea, listarea se face ascendent sau descendent, ținând cont de
 * valoarea câmpului m_nListareDesc din structura de tip sOpțiuni.
 * După selectarea tipului de sortare și a modului de listare, pointerul 'p' ia
 * valoarea unei funcții specializate pt fiecare tip de sortare și listare, apoi
 * se apelează qsort, cu datele ce trebuie sortate preluate din pVLst și cu
 * pointerul 'p' pentru funcția de comparare.
 * Funcțiile sunt: nListSortNumeAsc(), nListSortDimAsc(), nListSortDataAsc(), sau
 * nListSortNumeDesc(), nListSortDimDesc(), nListSortDataDesc().
 * Apelată de: main().
 * Parametri:
 *      - pOptCLI: pointer la obiectul tip sOptiuni care conține opțiunile
 *                 pentru efectuarea căutarii și afișare;
 *      - pVLst:   pointer la obiectul tip sVectorListare ce conține vectorii
 *                 alocați pentru listare.
 * Return: none.
 */
void vSorteazaVectorListare(OptCLI* pOptCLI, VLst* pVLst)
{
int nTipSortare;
int nListareDesc;
/* Pointer la funcție de comparare, în funcție de valorile din câmpurile
 * m_nTipSortare și m_nListareDesc din pOpt.
 */
int (*p)(const void*, const void*);

    nTipSortare = pOptCLI->m_nTipSortare;
    nListareDesc = pOptCLI->m_nListareDesc;

    if(nTipSortare == TIPSORTARE_NUME)            // După nume
    {
        if(!nListareDesc)           // Ascendent
            p = nListSortNumeAsc;
        else
            p = nListSortNumeDesc;  // Descendent
    }
    else if (nTipSortare == TIPSORTARE_DIMENSIUNE)      // După dimensiune
    {
        if(!nListareDesc)           // Ascendent
            p = nListSortDimAsc;
        else
            p = nListSortDimDesc;  // Descendent
    }
    else if (nTipSortare == TIPSORTARE_DATA)      // După dată
    {
        if(!nListareDesc)           // Ascendent
            p = nListSortDataAsc;
        else
            p = nListSortDataDesc;  // Descendent
    }
    else    // Fallback
    {
        p = nListSortNumeAsc;
    }

    qsort(pVLst->m_pLstFis, pVLst->m_nNrObAdaugate, sizeof(LstFis*), p);

}//vSorteazaVectorListare
/*****************************************************************************/
/* Funcția realizează printarea finală a listei de obiecte de tip fișier și /
 * sau director care au fost filtrate de funcția de căutare.
 * Funcția folosește un vector alocat dinamic de dimensiune FILENAME_MAX, care
 * va fi folosit pentru compunerea textului de afișat.
 * Sunt două formate folosite, szFormatFisier și szFormatDirector, în funcție
 * de tipul de obiect listat. Acest tip este dat de câmpul m_nTipFisier din
 * pointerul pVlst.
 * Funcția străbate tot vectorul de char*, în lungime de m_nNrObAdaugate și,
 * pentru fiecare în parte, listeaza, în funcție de tip, următoarele:
 *      - dacă este fișier normal:  <denumire> [F] <dimensiune> <data fișier>;
 *      - dacă este director:       <denumire> [D] <data director>.
 * Dacă la malloc() apar erori, funcția iese imediat cu cod de  eroare ERR_ALOCARE,
 * se ajunge în main() unde sunt eliberate toate resursele cu vDezalocaTot().
 * Apelată de: main().
 * Parametri:
 *      - pVLst: pointer la obiectul tip sVectorListare ce conține vectorii alocați
 *               pentru listare.
 * Return:
 *      0, dacă nu sunt erori de alocare sau copiere;
 *      ERR_ALOCARE, dacă sunt erori de aloare.
 */
int nListeazaVectorListare(VLst* pVLst)
{
int nRet;
char* szFormatFisier = "%s [F] %ld %s";
char* szFormatDirector = "%s [D] %s";
char* pszStringListare;
int nTipFisier;
int i;
    nRet = 0;
    nTipFisier = i = 0;

    pszStringListare = (char*)malloc(FILENAME_MAX);
    if(!pszStringListare)
    {
        fprintf(stderr, "Nu se poate aloca memorie pentru pszTmp Listare\n");
        nRet = ERR_ALOCARE;
        goto fExit;
    }
    for(i = 0; i < pVLst->m_nNrObAdaugate; i++)
    {
        memset(pszStringListare, 0, FILENAME_MAX);

        nTipFisier = pVLst->m_pLstFis[i]->m_nTipFisier;
        if (nTipFisier == TIPFISIER_TOT || nTipFisier == TIPFISIER_FIS)
        {
            snprintf(pszStringListare, FILENAME_MAX, szFormatFisier, pVLst->m_pLstFis[i]->m_pszNumeFis,
                     pVLst->m_pLstFis[i]->m_lDimFis, ctime(&pVLst->m_pLstFis[i]->m_tDataFis));
        }
        else if(nTipFisier == TIPFISIER_DIR)
        {
            snprintf(pszStringListare, FILENAME_MAX, szFormatDirector, pVLst->m_pLstFis[i]->m_pszNumeFis,
                     ctime(&pVLst->m_pLstFis[i]->m_tDataFis));
        }
        printf("%s", pszStringListare);
    }

    free(pszStringListare);

fExit:

return nRet;
}//nListeazaVectorListare
/*****************************************************************************/
/* Funcția afișează un mesaj conținând numele și versiunea programului.
 * Apelată de: main().
 * Parametri: none.
 * Return: none.
 */
void vArataVersiunea()
{
    printf("%s versiune: %s\n", APP_NUME, APP_VERSIUNE);
}//vArataVersiunea
/*****************************************************************************/
