public class main {
   public static void main(String argv[]) {
     System.loadLibrary("qepcad");
     qepcad.PRINTBANNER(); // This can be omitted.
     // Set the number of Saclib cells and the timeout.
     qepcad.mainLIB(5000000, 5);
     System.out.println("Successful initialization.");
     // First example: Check if there is a real number "a" that is 0. (TRUE)
     String input = "[]\n" + 
                        "(a)\n" +
                        "0\n" +
                        "(Ea)[a=0].\n" +
                        "finish\n";
     String output = qepcad.PCLIB(input);
     System.out.println(output);
     // Second example. Check if for each real number "b" b=1 holds. (FALSE)
     input = "[]\n" + 
                        "(b)\n" +
                        "0\n" +
                        "(Ab)[b>1].\n" +
                        "finish\n";
     output = qepcad.PCLIB(input);
     System.out.println(output);
   }
 }
