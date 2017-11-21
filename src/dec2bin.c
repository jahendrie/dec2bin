/*******************************************************************************
 * dec2bin.c    |   version 1.5     |   FreeBSD License     |   2017-11-20
 * James Hendrie                    |   hendrie.james@gmail.com
 *
 *  Description:
 *      Converts decimal numbers (positive integers) to binary numbers
 *
 *  Limitations:
 *      Can't handle numbers greater than (1e16) - 2
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_STRING_LENGTH 256
#define VERSION "1.5"


/*  ------------    Global Options  --------------- */
int dec2dec;                //  Decimal to decimal 'conversion'
int dec2bin;                //  Decimal to binary conversion
int dec2hex;                //  Decimal to hexadecimal conversion
int hexCaps;                //  Same, with capital letters
int dec2pHex;               //  Decimal to 'prefice' hexadecimal conversion
int phexCaps;               //  Same, with capital letters
int dec2oct;                //  Decimal to octal conversion
int textMode;               //  Text to binary conversion
int totalConversions;       //  Total number of conversion types to perform
int sections;               //  Whether to use sections / how many to use
int lineSpacing;            //  Whether to use extra line spacing
int verbose;                //  Verbosity
int bigEndian;              //  Big endian (1=yes, default)

/*  Optstring
 *      v   verbosity
 *      d   decimal
 *      b   binary
 *      x   hexidecimal
 *      X   hex with capital letters
 *      a   precise hex (converted from double without casting)
 *      A   same as above, but with captial letters
 *      o   octal
 *      e   little endian
 *      E   big endian (default)
 *      t   turns on 'text mode' conversion
 *      s   sections; optarg is number of chars between spaces.  Default is 0
 *      l   line spacing; lots of output separates results with an extra line
 *      h   help
 */
static const char *optString = "vdbxXaAoslhteE";



/*  ------------------  mem_error   ---------------------------
 *
 *  generic memory error message, takes const char *description as arg
 *  to print a slightly more specific message (if available)
 */
void mem_error( const char *description )
{
    fprintf(stderr, "ERROR:  Out of memory\n");
    fprintf(stderr, "%s\n", description );
}


/*  -----------------   print_usage ----------------------------
 *
 *  print program usage to file pointer *fp
 */
void print_usage( FILE *fp )
{
    fprintf(fp, "Usage:\tdec2bin [OPTIONS] NUMBER[s]\n");
}


/*  ----------------    print_help  -------------------------------
 *
 *  print the help
 */
void print_help( FILE *fp )
{
    print_usage(fp);

    fprintf(fp, "\nThis program converts positive integers to binary, octal, ");
    fprintf(fp, "hexadecimal and more.\n");

    fprintf(fp, "\nOptions:\n");
    fprintf(fp, "  -h or --help\tPrint this help text\n");
    fprintf(fp, "  --version\tPrint version and author info\n");
    fprintf(fp, "  -\t\tRead from stdin (absence of args will also work)\n");
    fprintf(fp, "  -v\t\tPrint number type before number (verbosity)\n");
    fprintf(fp, "  -b\t\tPrint binary (default)\n");
    fprintf(fp, "  -d\t\tPrint decimal\n");
    fprintf(fp, "  -o\t\tPrint octal\n");
    fprintf(fp, "  -x\t\tPrint hexadecimal\n");
    fprintf(fp, "  -X\t\tPrint hexadecimal with capital letters\n");
    fprintf(fp, "  -a\t\tPrecise hexadecimal (printf %%a, see 'man 3 printf')\n");
    fprintf(fp, "  -A\t\tPreceise hexadecimal with capital letters\n");
    fprintf(fp, "  -e\t\tPrint binary numbers as little-endian\n" );
    fprintf(fp, "  -E\t\tPrint binary numbers as big-endian (default)\n" );
    fprintf(fp, "  -t\t\tSwitch on 'text conversion' mode\n");
    fprintf(fp, "  -l\t\tPrint a line between sections of output\n");
    fprintf(fp, "  -s\t\tPrint in 4-character sections, space-separated\n");
    fprintf(fp, "\t\t(in text conversion mode, this will instead print 4");
    fprintf(fp, " spaces \n\t\tbetween each converted character)\n");
}


void print_version(void)
{
    printf("dec2bin version %s\n", (VERSION) );
    printf("James Hendrie - hendrie.james@gmail.com\n");
}


/*  ---------------------   power_of    --------------------------------
 *
 *  multiply [multiple] to the power of [power]
 */
double power_of( double multiple, int power )
{
    /*  If we're multiplying to the power of zero */
    if( power == 0 )
        return(1);

    double current = multiple;

    /*  For loop; make my number, GROW!!!! */
    int temp = 0;
    for( temp = 1; temp < power; ++temp )
        current *= multiple;

    /*  Return finished number */
    return( current );
}


/*  ----------------    get_array_size  -----------------------------------
 *
 *  we get the number of bits used for our binary number
 */
int get_array_size( double *userNumber )
{
    int arraySize = 0;          //  Size of our character array
    double temp = *userNumber;  //  We use this so that we don't kill userNumber

    while( (int)temp != 0 )     //  Make sure to cast as an int
    {
        temp = temp / 2;
        ++arraySize;
    }

    return( arraySize );
}


/*  ----------------------  print_number_string ----------------------------
 *
 *  print a number string, formatted as necessary
 */
void print_number_string( char *s, int arraySize )
{
    int i = 0;
    int spaces = 0;

    /*  If we're printing in sections */
    if( sections != 0 )
    {
        /*  Pad with as many zeroes as we need to get sections of 4 chars */
        for( i = 0; i < 4 - ( arraySize % 4); ++i )
        {
            putchar('0');
            ++spaces;
        }
        /*  When we hit our 'sections' limit, print a space */
        for( i = 0; i < arraySize; ++i )
        {
            if( spaces == 4 )
            {
                putchar(' ');
                spaces = 0;
            }

            putchar(s[i]);
            ++spaces;
        }

        printf("\n");   //  Print a newline at the end for readability
    }
    else
    {
        /*  Print our damn number string dammit!!!!!!!!!!!! ARRRARAGH */
        for( i = 0; i < arraySize ; ++i)
        {
            putchar(s[i]);
        }

        printf("\n");
    }

}




/*  ---------------------   create_binary_string     -----------------------
 *
 *  takes decimal value [userNumber] and creates a string of binary numbers
 *  that equal it; after this, we send that string to the print_number_string
 *  function
 */
char* create_binary_string( char *s, int arraySize, double *userNumber )
{
    memset( s, '\0', arraySize);

    int i = 0;                      //  Index in the string
    int multiple = arraySize - 1;   //  We'll use this for multiplication

    /*
     * Go through the string and, where appropriate, add a 1 or 0
     */

    //  If we're going big-endian
    if( bigEndian == 1 )
    {
        for( i = 0; i < arraySize+1; ++i )
        {
            /*  If our result is too high, we go further to the right */
            if( power_of( 2.0f, multiple) > *userNumber ) { s[i] = '0'; } 

            /*  Otherwise, we fill in the 'blanks' with 1s */
            else { s[i] = '1'; *userNumber -= power_of( 2.0f, multiple ); }

            --multiple;     //  Move to the right
        }
    }

    //  Otherwise, if little-endian
    else
    {
        for( i = ( arraySize - 1 ); i >= 0; --i )
        {
            /*  If our result is too high, we go further to the right */
            if( power_of( 2.0f, multiple) > *userNumber ) { s[i] = '0'; }

            /*  Otherwise, we fill in the 'blanks' with 1s */
            else { s[i] = '1'; *userNumber -= power_of( 2.0f, multiple ); } 

            --multiple;     //  Move to the right
        }
    }

    /*
     * Here I trim the string of leading zeroes, as I'm too lazy to fix the
     * program so that there are no leading zeroes to begin with
     */
    if( *userNumber != 0 )      //  Make sure user didn't actually give us a 0
    {
        while( s[0] == '0' ) { ++s; }
    }

    /*  Send to print_number_string function */
    return( s );
}



/*  Print a string from a number passed to it */
int string_printer( double *userNumber )
{

    /*  Determines array size for our string */
    int arraySize = get_array_size( userNumber );

    /*  Make sure we have an array size to work with */
    if( arraySize < 1 )
    {
        if( *userNumber < 0 )
        {
            printf("What is this, a joke!?\n");
            return(1);
        }

        else if( *userNumber == 0 )
            arraySize = 1;
    }

    /*
     * Create a string pointer, and then allocate a number of bytes to it
     * according to the value given by the previous function
     */
    char *s = malloc( arraySize );
    if( s == NULL )
    {
        mem_error("Main:  Allocating memory to string buffer");
        return(1);
    }


    if( dec2dec == 1 )      //  Decimal output (for whatever reason)
    {
        if( verbose == 1 )
            printf("DEC\t");

        sprintf( s, "%s%d", s, (unsigned int)*userNumber );
        print_number_string( s, strlen(s) );
        memset( s, '\0', arraySize + 1 );   //  Clean up string
    }

    if( dec2hex == 1 )      //  Regular hex (non-capital)
    {
        if( verbose == 1 )
            printf("HEX\t");

        sprintf( s, "%s%x", s, (unsigned int)*userNumber);
        print_number_string( s, strlen(s) );
        memset( s, '\0', arraySize + 1 );   //  Clean up string

    }

    if( hexCaps == 1 )      //  Regular hex (captial letters)
    {
        if( verbose == 1 )
            printf("HEX\t");

        sprintf( s,"%s%X", s, (unsigned int)*userNumber);
        print_number_string( s, strlen(s) );
        memset( s, '\0', arraySize+1 );   //  Clean up string
    }

    if( dec2pHex == 1 )     //  SUPER HEX (non-caps)
    {
        if( verbose == 1 )
            printf("0xHEX\t");

        sprintf( s, "%s%a", s, *userNumber );
        print_number_string( s, strlen(s) );
        memset( s, '\0', arraySize + 1 );
    }

    if( phexCaps == 1 )     //  SUPER HEX (caps)
    {
        if( verbose == 1 )
            printf("0xHEX\t");

        sprintf( s, "%s%A", s, *userNumber );
        print_number_string( s, strlen(s) );
        memset( s, '\0', arraySize + 1 );
    }

    if( dec2oct == 1 )  //  Octal
    {
        if( verbose == 1 )
            printf("OCT\t");

        sprintf( s, "%s%o", s, (unsigned int)*userNumber );
        print_number_string( s, strlen(s) );
        memset( s, '\0', arraySize + 1 );
    }


    /*  We bail if dec2bin is 0 */
    if( dec2bin == 0 )
        return(0);


    /*  If verbose, print BIN */
    if( verbose == 1 )
        printf("BIN\t");

    /*  Create a binary string, then print it */
    s = create_binary_string( s, arraySize, userNumber );
    print_number_string( s, arraySize );

    /*  Free memory, null pointer */
    free( s );
    s = NULL;

    return(0);
}


void text_to_number_finisher( char *tmpString )
{
    printf( "%s", tmpString);

    /*  Check if the user wants to print lines between each char */
    if( lineSpacing == 1 )
    {
        printf("\n");
    }
    else if( sections != 0 )    //  User wants 'sections' instead
    {
        int sCount;     //  Second counter.  Yeah, I suck at names.
        for( sCount = 0; sCount < sections; ++sCount )
            printf(" ");
    }

    memset( tmpString, '\0', 8 );   //  Null out the string
}


int text_to_number( const char *string )
{
    int counter = 0;    //  Generic counter
    double userNumber;  //  Our user number (converted from ascii char )
    int pCount = 0;     //  Number of strings printed

    /*  Create our string pointer, allocate memory to it */
    char *tmpString;
    tmpString = malloc( 8 );
    if( tmpString == NULL )
    {
        mem_error("In:  text_to_number");
        return( 1 );
    }
    memset( tmpString, '\0', 8 );   //  Null out the string

    for( counter = 0; counter < strlen( string ); ++counter )
    {
        /*
         * If our 'character' isn't ASCII, we skip it.
         */
        if( isascii( string[counter] ) )
        {
            if( pCount > 0 && lineSpacing != 0 && totalConversions > 1 )
            {
                printf("\n");
            }

            /*  Get the value of the character, convert it to a binary string */
            userNumber = string[counter];

            if( dec2bin == 1 )
            {
                if( verbose == 1 ) printf( "%c  BIN    ", string[counter] );

                double uNumber = userNumber;
                tmpString = create_binary_string( tmpString, 8, &uNumber );
                tmpString[8] = '\0';    //  Cut the extra 0 off the end
                text_to_number_finisher( tmpString );
            }

            if( dec2hex == 1 )
            {
                if( verbose == 1 ) printf( "%c  HEX    ", string[counter] );

                sprintf( tmpString, "%x", (unsigned int)userNumber );
                text_to_number_finisher( tmpString );
            }

            if( hexCaps == 1 )
            {
                if( verbose == 1 ) printf( "%c  HEX    ", string[counter] );

                sprintf( tmpString, "%X", (unsigned int)userNumber );
                text_to_number_finisher( tmpString );
            }

            if( dec2pHex == 1 )
            {
                if( verbose == 1 ) printf( "%c  0xHEX  ", string[counter] );

                sprintf( tmpString, "%a", userNumber );
                text_to_number_finisher( tmpString );
            }

            if( phexCaps == 1 )
            {
                if( verbose == 1 ) printf( "%c  0xHEX  ", string[counter] );

                sprintf( tmpString, "%A", userNumber );
                text_to_number_finisher( tmpString );
            }

            if( dec2oct == 1 )
            {
                if( verbose == 1 ) printf( "%c  OCT    ", string[counter] );

                sprintf( tmpString, "%o", (unsigned int)userNumber );
                text_to_number_finisher( tmpString );
            }

            if( dec2dec == 1 )
            {
                if( verbose == 1 ) printf( "%c  DEC    ", string[counter] );

                sprintf( tmpString, "%d", (unsigned int)userNumber );
                text_to_number_finisher( tmpString );
            }



            ++pCount;   //  Increment our print counter
        }
    }

    free( tmpString );
    tmpString = NULL;

    return( 0 );
}


/*
 * This function exists so that people can just pipe numbers to the program
 * and have it work on them
 */
void string_send_stdin(void)
{
    char *theNumber = NULL;     //  Used for strtok
    char line[256];
    double userNumber = 0;
    int pCount = 0;

    while( fgets(line, 256, stdin) != NULL )
    {
        /*
         * We'll use strtok to tokenize the string, number by number, using the
         * space character as a delimiter
         */
        theNumber = strtok(line, " ");
        while( theNumber != NULL )
        {
            /*  If the user wants slightly prettier output */
            if( lineSpacing == 1 && pCount > 0 )
            {
                printf("\n");
            }

            if( textMode == 1 )
            {
                char tempString[256];
                char *t = tempString;

                if( pCount > 0 )
                {
                    sprintf( tempString, " %s", theNumber );
                    text_to_number( t );
                }
                else
                    text_to_number( theNumber );
            }
            else
            {
                userNumber = atof( theNumber );
                string_printer( &userNumber );
            }

            theNumber = strtok(NULL, " ");  //  Re-run strtok, grab next number

            ++pCount;
        }
    }

    /*  If we're using text conversion mode, we do one last readability check */
    if( textMode == 1 && lineSpacing == 0 && pCount > 0 )
        printf("\n");
}



/*  ============================    MAIN    ==================================*/
int main( int argc, char *argv[] )
{

    if( argc >= 2 )
    {
        if( strcmp( "-h", argv[1] ) == 0 || strcmp( "--help", argv[1] ) == 0 )
        {
            print_help(stdout);
            return(0);
        }

        if( strcmp( "--version", argv[1]) == 0 )
        {
            print_version();
            return(0);
        }
    }

    /*  Init variables */
    int opt = 0;

    verbose = 0;
    dec2dec = 0;
    dec2bin = 0;
    dec2hex = 0;
    dec2pHex = 0;
    dec2oct = 0;
    hexCaps = 0;
    phexCaps = 0;
    sections = 0;
    lineSpacing = 0;
    textMode = 0;
    totalConversions = 0;
    bigEndian = 1;

    /*  Do the optString thing */
    opt = getopt( argc, argv, optString );
    while( opt != -1 )
    {
        /*  Check and see if we got a wise-aleck */
        if( ! isalpha(opt) )
        {
            return(1);
        }

        switch( opt )
        {
            case 'v':   //  Verbosity
                verbose = 1;
                break;
            case 'd':   //  Decimal output on
                dec2dec = 1;
                break;
            case 'b':   //  Binary on (default)
                dec2bin = 1;
                break;
            case 'a':   //  Precise hex w/out casting
                dec2pHex = 1;
                break;
            case 'A':   //  Precise hex w/out casting (using captial letters)
                phexCaps = 1;
                break;
            case 'x':   //  Hex, cast to unsigned int
                dec2hex = 1;
                break;
            case 'X':   //  Hex, cast to unsigned int, with captial letters
                hexCaps = 1;
                break;
            case 'o':   //  Octal
                dec2oct = 1;
                break;
            case 's':   //  Print output in sections of 4 separated by spaces
                sections = 4;
                break;
            case 'l':   //  If more than one number received, print extra lines
                lineSpacing = 1;
                break;
            case 'h':   //  Print help
                print_help(stdout);
                return(0);
                break;
            case 't':   //  Turn on textmode conversion
                textMode = 1;
                break;
            case 'e':   //  little endian
                bigEndian=0;
                break;
            case 'E':   //  big endian
                bigEndian=1;
                break;

            default:
                /*  Won't be seeing this, disregard */
                break;
        }

        opt = getopt( argc, argv, optString );
    }

    /*  Once outside the loop, deal with argc and argv */
    argv += (optind - 1);
    argc -= (optind - 1);

    /*  If no options specified, the default is to print binary */
    if( dec2bin == 0 && dec2hex == 0 && dec2oct == 0 && dec2pHex == 0 &&
            hexCaps == 0 && phexCaps == 0 && dec2dec == 0 )
    {
        dec2bin = 1;
    }


    /*  Here, we find the total number of conversions... for reasons */
    totalConversions = ( dec2bin + dec2dec + dec2hex + dec2pHex + dec2oct +
            hexCaps + phexCaps );

    /*  Check if the user's trying to use stdin */
    if( argc <= 1 || strcmp( "-", argv[1] ) == 0 )
    {
        string_send_stdin();
        return(0);
    }

    int pCount = 0;     //  Used to count the number of sections printed
    while( argc > 1 )
    {
        /*  If the user wants slightly prettier output */
        if( lineSpacing == 1 && pCount > 0 )
        {
            printf("\n");
        }

        /*  Convert a string to a number */
        if( textMode == 1 )
        {
            if( ( text_to_number( argv[1] )) == 1 )
                return( 1 );

            ++pCount;
            ++argv;
            --argc;

            /*
             * If we've printed more than once and they did not explicitly
             * enable line prettification, we still end the output with a
             * newline character.
             */
            if( lineSpacing == 0 && pCount > 0 )
                printf("\n");

            continue;
        }

        double userNumber = atof(argv[1]);  //  Create a new double from argv[1]
        if( string_printer( &userNumber ) == 1 )    //  Send to string_printer
            return(1);

        /*  Our various counters and lists */
        ++pCount;
        ++argv;
        --argc;
    }

    return(0);
}
