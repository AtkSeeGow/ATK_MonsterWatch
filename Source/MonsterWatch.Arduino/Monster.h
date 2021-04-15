// Monster.h

#ifndef _MONSTER_h
#define _MONSTER_h

class Monster {
  public:
    Monster();
    void SetAction(int action);
    void Execution();
  private:
    int action;
};

#endif
