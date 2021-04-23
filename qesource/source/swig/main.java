public class main {
   public static void main(String argv[]) {
     System.loadLibrary("qepcad");
     qepcad.PRINTBANNER();
     qepcad.mainLIB(5);
     System.out.println("Successful initialization.");
   }
 }
