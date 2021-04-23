public class main {
   public static void main(String argv[]) {
     System.loadLibrary("qepcad");
     qepcad.PRINTBANNER();
     // Set the cells and the timeout.
     qepcad.mainLIB(5000000, 5);
     System.out.println("Successful initialization.");
     String input = "[]\n" + 
                        "(a)\n" +
                        "0\n" +
                        "(Ea)[a=0].\n" +
                        "go\n" + "go\n" + "go\n" + "sol T\n";
     System.out.println(qepcad.PCLIB(input));
   }
 }
