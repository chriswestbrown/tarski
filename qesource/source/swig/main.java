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
     // Third example. Getting only the answer without the introductory questions.
     String formula = "[]\n(x,y,z)\n2\n(Ez)[x^2 + y^2 + z^2 < 1 /\\ x + y + z > 0].\n";
     String assumption = "[y > 0]";
     System.out.println(qepcad.SLFQLIB(formula, ""));
     System.out.println(qepcad.SLFQLIB(formula, assumption));

     qepcad.ENDQEPCADLIB(); // not necessary
   }
 }
