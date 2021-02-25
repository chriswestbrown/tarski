/*======================================================================
                            HELPFRD()

Help file read.
======================================================================*/
#include "qepcad.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define HELPPATH               "qepcad.help"
#define MaxDescriptionSize     6000
#define MaxCommandNameSize     60
#define MaxNumInteractionLocs  30
#define MaxNumClassifications  30
extern char *Helps[]; 

void HELPFRD()
{
       FILE *fp;
       char *qepath;
       char helppath[1000];  /* Should be enough. */
       char buffer[MaxDescriptionSize]; 
       char name[MaxCommandNameSize];
       char location[MaxNumInteractionLocs];  
       char classification[MaxNumInteractionLocs];  
       unsigned int size;
       char *p;
       Word c;
       Word  N,I,L,T,C,D;
       int r1, r2, r3, r4, r5; /* fscanf return values. */
  
Step1: /* Open the text file containing the helps. */
       if ((qepath = getenv("qe")) == NULL)
          strcpy(helppath,HELPPATH);
       else {
          strcpy(helppath,qepath);
          strcat(helppath,"/bin/qepcad.help"); }
       if (!(fp = fopen(helppath,"r"))) {
         fprintf(stderr,"Error HELPFRD: Could not open %s\n",helppath);
         exit(1); }

Step2: /* Read in the main help text and echo it. */
       c = getc(fp);
       while (c != '@') {
         putchar(c);
         c = getc(fp); }

Step3: /* Setup for reading help items. */
       COMMANDS = NIL;
       COMCLASS = NIL;

Step4: /* Read in the command name, index, location and classification. */
       r1 = fscanf(fp,"%s", name);
       if (r1 < 1) { FAIL("HELPFRD","Invalid helpfile syntax!"); }
       if (strcmp(name,"@@@") == 0) goto Step6;
       N = LFS(name);
       r2 = fscanf(fp,"%d", &I);
       if (r2 < 1) { FAIL("HELPFRD","Invalid helpfile syntax!"); }
       r3 = fscanf(fp,"%s", location);
       if (r3 < 1) { FAIL("HELPFRD","Invalid helpfile syntax!"); }
       L = LFS(location);
       r4 = fscanf(fp,"%s",classification);
       if (r4 < 1) { FAIL("HELPFRD","Invalid helpfile syntax!"); }
       T = LFS(classification);
       C = LIST4(N,I,L,T);
       COMMANDS = COMP(C,COMMANDS);

Step5: /* Read in the help text for the command. */
       p = buffer;
       while ((c=getc(fp)) != '@');
       while ((c=getc(fp)) != '@') *p++ = c;
       *p = '\0';
       size = p - buffer;
       Helps[I] = (char *)malloc(size + 5);
       strcpy(Helps[I],buffer); 
       goto Step4;

Step6: /* Read in classification descriptions. */
       r5 = fscanf(fp,"%s",classification);
       if (r5 < 1) { FAIL("HELPFRD","Invalid helpfile syntax!"); }
       if (strcmp(classification,"@@@") == 0) goto Step7;
       T = LFS(classification);
       p = buffer;
       while ((c=getc(fp)) != '@');
       while ((c=getc(fp)) != '@') *p++ = c;
       *p = '\0';
       D = LFS(buffer);
       COMCLASS = COMP(LIST2(T,D),COMCLASS);
       goto Step6;

Step7: /* Done.  */
       COMMANDS = INV(COMMANDS);
       COMCLASS = INV(COMCLASS);
       fclose(fp);

Return: /* Prepare for return. */
       return;
}
