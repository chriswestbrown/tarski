(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun pi () Real)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(assert (and (>  (+ (- 542038508653702546896624539949334927426922000109643764353707923238298996227330525297976868873728) (* pi 963175758274704302623500825683126850919390382695778691926168092409162768139361987575146718290525)) 0) (<= (* (- 1) skoY) 0) (> (* (- 1) (+ (- 31415927) (* pi 10000000))) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (= (* (- 1) (+ (+ 277555600 (* skoY (* skoY (- 1)))) (* skoX (* skoX (+ 15328072984 (* skoX (* skoX (+ 129098541721 (* skoX (* skoX (+ 21404723599 (* skoX (* skoX (+ 1024027285 (* skoX (* skoX 15132100)))))))))))))))) 0) (=  (+ (- 1325421053866224634595698711821825) (* skoY (* skoY 295147905179352825856))) 0)))
(check-sat)