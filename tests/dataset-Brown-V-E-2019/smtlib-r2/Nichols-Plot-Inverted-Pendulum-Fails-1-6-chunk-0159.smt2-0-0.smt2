(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun pi () Real)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(assert (and (<  (+ (* pi (- 5440946605830286186600119779896676312043382731194639204620328518820944398383889135101594508785596725)) (* skoY (+ (- 526264639498861910375974270503146085801042887605878095137540261970970183407317686603535613952) (* pi 2239054049774517052038495214959290392973727957829476820560235904957134323381651627174369689600)))) 0) (=  (+ (- 1325421053866224634595698711821825) (* skoY (* skoY 295147905179352825856))) 0) (=  (+ (+ 277555600 (* skoY (* skoY (- 1)))) (* skoX (* skoX (+ 15328072984 (* skoX (* skoX (+ 129098541721 (* skoX (* skoX (+ 21404723599 (* skoX (* skoX (+ 1024027285 (* skoX (* skoX 15132100))))))))))))))) 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (=  skoY 0)))
(check-sat)