class Property
{
public:
  virtual const string& getName() const;
  virtual bool isIrreducible() { return true; }
  virtual bool isCheck() { return false; }
};
class NirProp : public Property
{
public:
  virtual bool isIrreducible() { return flase; }
};
class Check : public Property
{
public:
  virtual bool isCheck() { return true; }
  virtual bool check(GoalContext& GC, IntPolyRef p);
};  
class NirCheck : public Property
{
public:
  virtual bool isCheck() { return true; }
  virtual bool check(GoalContext& GC, FactRef F);
};  
