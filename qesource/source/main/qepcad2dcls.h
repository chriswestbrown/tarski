
class QepcadCls2D : public QepcadCls
{
public:
  QepcadCls2D(Word V, Word Fs) : QepcadCls(V,Fs) { }
  QepcadCls2D() : QepcadCls() { }
  void CONSTRUCT(Word c,Word k,Word f,Word Ps_,Word As);
  void PROJECTauto(Word r, Word A,Word *P_, Word *J_);
  Word TICADauto(Word Q,Word F,Word f,Word P,Word A);
};

