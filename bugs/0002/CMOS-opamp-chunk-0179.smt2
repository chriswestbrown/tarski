(set-info :smt-lib-version 2.6)
(set-logic QF_NRA)
(set-info :source |
These benchmarks used in the paper:

  Dejan Jovanovic and Leonardo de Moura.  Solving Non-Linear Arithmetic.
  In IJCAR 2012, published as LNCS volume 7364, pp. 339--354.

The meti-tarski benchmarks are proof obligations extracted from the
Meti-Tarski project, see:

  B. Akbarpour and L. C. Paulson. MetiTarski: An automatic theorem prover
  for real-valued special functions. Journal of Automated Reasoning,
  44(3):175-205, 2010.

Submitted by Dejan Jovanovic for SMT-LIB.


|)
(set-info :category "industrial")
(set-info :status sat)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(declare-fun pi () Real)
(assert (and (not (<= (* skoY (* skoY (+ (* skoX (* skoX (+ (- 1800000000000000000000000) (* skoX (* skoX (- 1800000000)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ 150000000000000000000000 (* skoX (* skoX 150000000))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (- 5000000000000000000000) (* skoX (* skoX (- 5000000)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (/ 625000000000000000000 7) (* skoX (* skoX (/ 625000 7)))))) (* skoY (* skoY (* skoX (* skoX (+ (/ (- 62500000000000000000) 63) (* skoX (* skoX (/ (- 62500) 63)))))))))))))))))))) (* skoX (* skoX (+ (- 3600000000000000000000000) (* skoX (* skoX (- 3600000000)))))))) (and (not (<= (* skoY (* skoY (+ (* skoX (* skoX (+ 3600060000000000000000000 (* skoX (* skoX 3600060000))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (- 1200005000000000000000000) (* skoX (* skoX (- 1200005000)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (/ 480000500000000000000000 3) (* skoX (* skoX (/ 480000500 3)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (/ (- 240000062500000000000000) 21) (* skoX (* skoX (/ (- 480000125) 42)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (/ 96000006250000000000000 189) (* skoX (* skoX (/ 384000025 756)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (/ (- 968750000000000000000) 63) (* skoX (* skoX (/ (- 968750) 63)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (/ 62500000000000000000 189) (* skoX (* skoX (/ 62500 189)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (/ (- 19726562500000000000) 3969) (* skoX (* skoX (/ (- 315625) 63504)))))) (* skoY (* skoY (+ (* skoX (* skoX (+ (/ 195312500000000000 3969) (* skoX (* skoX (/ 3125 63504)))))) (* skoY (* skoY (* skoX (* skoX (+ (/ (- 9765625000000000) 35721) (* skoX (* skoX (/ (- 625) 2286144))))))))))))))))))))))))))))))))))) (+ (- 970000000000000000000000000000) (* skoX (* skoX (+ 120000000000000000000 (* skoX (* skoX 3600119999)))))))) (and (not (<= pi (/ 15707963 5000000))) (and (not (<= (/ 31415927 10000000) pi)) (and (<= skoY (* pi (/ 1 3))) (and (<= (* pi (/ 1 4)) skoY) (and (<= skoX 120) (<= 100 skoX)))))))))
(check-sat)
(exit)
