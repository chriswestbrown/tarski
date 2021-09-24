public class main {
   public static void main(String argv[]) {
     System.out.println("Loading library...");
     System.loadLibrary("tarski");
     // Set the number of Saclib cells and the timeout.
     System.out.println("Calling mainLIB...");
     tarski.mainLIB(5000000, 5);
     System.out.println("Successful initialization.");
     // First example: Check if there is a real number "x" that is greater than 0. (TRUE)
     String input = "(qepcad-api-call [ex x [x > 0]])";
     String output = tarski.PCLIB(input);
     System.out.println(output);
     // Second example: Check if there are real numbers "x" and "y" such that x^2*y^2<0. (TRUE)
     input = "(qepcad-api-call [ex x,y [x^2 y^2 < 0]])";
     output = tarski.PCLIB(input);
     System.out.println(output);
     input = "(def epc (lambda (F) (def L (getargs F)) (def V (get L 0 0 1)) (def B (bbwb (get L 1))) (if (equal? (get B 0) 'UNSAT) [false] ((lambda () (def G (qfr (t-ex V (get B 1)))) (if (equal? (t-type G) 1) G (if (equal? (t-type G) 6) (qepcad-api-call G 'T) (if (equal? (t-type G) 5) (qepcad-api-call (bin-reduce t-or (map (lambda (H) (qepcad-api-call (exclose H '(m)) 'T)) (getargs G))) 'T) (qepcad-api-call G 'T))))))))) (def expand (lambda (F) (def A (getargs F)) (def V (get A 0 0 1)) (def G (get A 1)) (def X (dnf G)) (def L (if (equal? (t-type X) 5) (getargs X) (list X))) (map (lambda (f) (exclose f '(m))) L) )) (epc [ ex v10,v11,v12,v13,v14,v9,w1 [v11>0 /\\ v12>0 /\\ v13>0 /\\ v14>0 /\\ v11+v12+v13-w1=0 /\\ -v10^2+v11^2-v9^2+2 v9-1=0 /\\ -v10^2+2 v10+v12^2-v9^2+2 v9-2=0 /\\ -v10^2+v13^2-v9^2=0 /\\ -m+w1=0 /\\ -v14+1=0]])";
     output = tarski.PCLIB(input);
     System.out.println(output);
     tarski.ENDTARSKILIB(); // not necessary
   }
 }
