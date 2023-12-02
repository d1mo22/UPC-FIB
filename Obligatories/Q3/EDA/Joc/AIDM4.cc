#include "Player.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME DM4


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }
  /*
    - Pioneers Exploradores | 50hp | 15 unidades | MAXIMO 60

    - Furyans Guerrreros | 100hp | 5 unidades | Atk 25-50 | MAXIMO 20
        NO pueden conquistar ni coger gemas

    - Hellhounds (bajotierra) | Inmortal | 3 unidades | Instakill (alrededor) |
        Siempre se acercan al explorador/guerrero mas cercano

    - Necromonger (superficie) | 75hp | Atk 20-40
        Spawnean con un 50% prob cada ronda en las celdas que acaban de volverse noche
          
    - Al principio de CADA ronda se RECUPERAN 5hp de todas las unidades
    - Cada celda puede tener una unidad, si una entra en el lugar de una se considera ataque
     
    - Cada ronda solo la primera orden sera usada
        Si se da mas de 1000 ordenes el la misma ronda ABORTA

    - El orden de accion de los jugadores es aleatorio, pero se guarda el orden de las unidades

    - Al matar exploradores/guerreros estos spawnean en un lugar random y pertenecen al grupo
      que los ha mantado. 
      A NO SER QUE los mate la maquina ENTONCES spawnean en un equipo random.
  */
  
  /*
    IDEAS:
      Como los guerreros no pueden conquistar celdas ni gemas
      pueden protejer los ascencores o acompanyar al explorador.
      Dar ordenes de mas urgente a menos urgente ya que se ejecutan en orden
  */

  /**
   * Types and attributes for your player can be defined here.
   */
  typedef pair<int,int> PI;
  typedef vector<int> VI;
  typedef vector<Cell> VC;
  typedef vector<bool> VB;
  typedef vector<VC> VVC;
  typedef vector<VI> VVI;
  typedef vector<VB> VVB;
  typedef vector<VVC> mapa;
  typedef vector<VVI> matrix;

  //Llegeix tota la informacio del mapa subterrani
  //Cave camino | Rock pared
 void llegir_mapa(mapa& mc, matrix& mi) {
    int n = rows();
    int t = cols();
    mc.resize(n);
    mi.resize(n);

    for (int i = 0; i < n; ++i) {
        mc[i].resize(t);
        mi[i].resize(t);

        for (int j = 0; j < t; ++j) {
            mc[i][j].resize(2);
            mi[i][j].resize(2);

            for (int k = 0; k < 2; ++k) {
                Cell c = cell(i, j, k);
                mc[i][j][k] = c;

                if (c.id != -1) {
                    Unit u = unit(c.id);
                    if (u.player != me()) {
                        mi[i][j][k] = u.type;
                    } else {
                        mi[i][j][k] = -1;
                    }
                } else {
                    mi[i][j][k] = -1;
                }
            }
        }
    }
}

  PI sol() {
    return {((40+2*round())%80),((79+2*round())%80)};
  }

  Dir desicio(const Pos& p, const Pos& next) {
    if (next.i > p.i and next.j > p.j) return BR;
    else if (next.i > p.i and next.j < p.j) return LB;
    else if (next.i < p.i and next.j > p.j) return RT;
    else if (next.i < p.i and next.j < p.j) return TL;
    else if (next.i == p.i and next.j > p.j) return Right;
    else if (next.i == p.i and next.j < p.j) return Left;
    else if (next.i > p.i and next.j == p.j) return Bottom;
    else if (next.i < p.i and next.j == p.j) return Top;
    else {
      Pos aux = p;
      aux.k = 1;
      if (daylight(aux)) return None;
      else if (p.k == 0) return Up;
      else return Down;
    }
  }

  int distancia(const Pos& p1, const Pos& p2) {
    return sqrt(pow(p1.i - p2.i,2) + pow(p1.j - p2.j, 2));
  }

  Pos buscar_ascensors(const mapa& m, const Pos& p) {
    VVB visitat(rows(), VB(cols(), false));
    queue<Pos> q;
    q.push(p);
    visitat[p.i][p.j] = true;

    while(!q.empty()) {
      Pos p = q.front();
      q.pop();

      int x = p.i;
      int y = p.j;

      if (m[x][y][0].type == Elevator) return p;

      for (int i = Bottom; i <= LB; ++i) {
        Pos next = p + Dir(i);
        if (pos_ok(next) && !visitat[next.i][next.j] && m[next.i][next.j][0].type != Rock) {
          q.push(next);
          visitat[next.i][next.j] = true;
        }
      }
    }
    return Pos(-1,-1,0);
  }

  Pos buscar_ascensors_superficie(const mapa& m, const Pos& p) {
    VVB visitat(rows(), VB(cols(), false));
    queue<Pos> q;
    q.push(p);
    visitat[p.i][p.j] = true;

    while(!q.empty()) {
      Pos p = q.front();
      q.pop();

      int x = p.i;
      int y = p.j;

      if (m[x][y][1].type == Elevator) return p;

      for (int i = Bottom; i <= LB; ++i) {
        Pos next = p + Dir(i);
        if (pos_ok(next) && !visitat[next.i][next.j]) {
          q.push(next);
          visitat[next.i][next.j] = true;
        }
      }
    }
    return Pos(-1,-1,0);
  }

  Pos conquistar(const mapa& m, const Pos& p) {
    VVB visitat(rows(), VB(cols(), false));
    queue<Pos> q;
    q.push(p);
    visitat[p.i][p.j] = true;
    vector<vector<Pos>> pare(rows(), vector<Pos>(cols(), Pos(-1,-1,-1)));

    while(!q.empty()) {
      Pos p = q.front();
      q.pop();

      int x = p.i;
      int y = p.j;

        if (m[x][y][0].type == Cave) {
            Cell c = cell(x, y, 0);
            if (c.owner != me() && c.owner != -1 && c.id == -1) {
                vector<Pos> path;
                while (p.i != -1) {
                    path.insert(path.begin(), p);
                    p = pare[p.i][p.j];
                }
                return path[1];
            } else if (c.owner != me() && c.id == -1 && c.owner == -1) {
                vector<Pos> path;
                while (p.i != -1) {
                    path.insert(path.begin(), p);
                    p = pare[p.i][p.j];
                }
                return path[1];
            }
        }
      vector<int> direccions = random_permutation(7);
      for (int i = 0; i <= 7; ++i) {
        Pos next = p + Dir(direccions[i]);
        if (pos_ok(next) && !visitat[next.i][next.j] && m[next.i][next.j][0].type != Rock) {
          q.push(next);
          visitat[next.i][next.j] = true;
          pare[next.i][next.j] = p;
        }
      }
    }
    return {};
  }

  Pos eliminar_exploradors(const matrix& mat, const Pos& p) {
    VVB visitat(rows(), VB(cols(), false));
    queue<Pos> q;
    q.push(p);
    visitat[p.i][p.j] = true;

    while (!q.empty()) {
      Pos p = q.front();
      q.pop();

      int x = p.i;
      int y = p.j;

      if (mat[x][y][0] == Pioneer) return p;

      for (int i = Bottom; i <= LB; ++i) {
        Pos next = p + Dir(i);
        if (pos_ok(next) && !visitat[next.i][next.j] && mat[next.i][next.j][0] != Rock) {
          q.push(next);
          visitat[next.i][next.j] = true;
        }
      }
    }
    return Pos(-1,-1,0);
  }

  Pos eliminar_guerrers(const matrix& mat, const Pos& p) {
    VVB visitat(rows(), VB(cols(), false));
    queue<Pos> q;
    q.push(p);
    visitat[p.i][p.j] = true;

    while (!q.empty()) {
      Pos p = q.front();
      q.pop();

      int x = p.i;
      int y = p.j;

      if (mat[x][y][0] == Furyan) return p;

      for (int i = Bottom; i <= LB; ++i) {
        Pos next = p + Dir(i);
        if (pos_ok(next) && !visitat[next.i][next.j] && mat[next.i][next.j][0] != Rock) {
          q.push(next);
          visitat[next.i][next.j] = true;
        }
      }
    }
    return Pos(-1,-1,0);
  }

  pair<bool, Pos> radar(const matrix& mat, const Pos& p) {
    int x = p.i;
    int y = p.j;

    for (int i = max(0, x-3); i < min(rows(), x+3); ++i) {
      for (int j = max(0,y-3); j < min(cols(), y+3); ++j) {
        if (mat[i][j][0] == Hellhound) return {true, Pos(i,j,0)};
      }
    }
    return {false, Pos(-1,-1,-1)};
  }

  Dir escapa(const mapa& m, const Pos& me, const Pos& enemic) {
    Pos nova;
    if (me.i > enemic.i) {
        if (me.j > enemic.j) {
            nova = me + BR;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return BR;

            nova = me + Right;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Right;
        } 
        else if (me.j == enemic.j) {
            nova = me + Right;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Right;

            nova = me + Left;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Left;
        } 
        else {
            nova = me + LB;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return LB;

            nova = me + Left;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Left;
        }
    } 
    else if (me.i == enemic.i) {
        if (me.j > enemic.j) {
            nova = me + Bottom;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Bottom;

            nova = me + BR;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return BR;
        } 
        else if (me.j < enemic.j) {
            nova = me + Top;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Top;

            nova = me + RT;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return RT;
        }
    } 
    else {
        if (me.j > enemic.j) {
            nova = me + RT;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return RT;

            nova = me + Top;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Top;
        } 
        else if (me.j == enemic.j) {
            nova = me + Left;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Left;

            nova = me + Right;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Right;
        } 
        else {
            nova = me + TL;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return TL;

            nova = me + Top;
            if (pos_ok(nova) && m[nova.i][nova.j][0].type != Rock) return Top;
        }
    }

    // Si no se puede escapar en ninguna dirección, devolver None
    return None;
}

  //Se mueve en una direccion que no esta conquistada
  void move_pionner(const mapa& m, const matrix& mat) {
    /* 
      Primero miramos si hay algun hellhound cerca (3x3)
      En el caso que no
        -Priorizamos las celdas que son de un enemigo antes que las que
        no tienen dueño, si no hay ninguna vamos a una que no sea nuestra
      En el caso que haya algun hellhoun cerca --> Huimos
    */

    VI expo = pioneers(me());
    for (int id : expo) {
      Unit u = unit(id);
      Pos e = u.pos;
      pair<bool, Pos> enemics = radar(mat, e);
      //Si esta sota terra
      if(!enemics.first) {
        Pos next = conquistar(m, e);
        command(id, desicio(e, next));
      } 
      else {
        command(id, escapa(m, e, enemics.second));
        cerr << "Posicio actual: " << e << endl;
        cerr << "Posicio enemic: " << enemics.second << endl;
      } 
    }
  }

  void move_furyans(const mapa& m, const matrix& mat) {
    /*
      Primero mirar si hay algun hellhound a una distancia de 3x3
      En el caso que no
        -Si la vida del guerrero es > 50 vamos a buscar otros guerreros
        -Si la vida es menor vamos a buscar exploradores hasta que nos recuperemos
      Si hay hellhounds cerca --> Huir de ellos
    */

    VI expo = furyans(me());
    for (int id : expo) {
      Unit u = unit(id);
      Pos e = u.pos;
      Pos next = eliminar_exploradors(mat, e);
      command(id, desicio(e, next));
    }
  }


  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {
    mapa m;
    matrix mat;
    llegir_mapa(m, mat);
    move_pionner(m, mat);
    move_furyans(m, mat);
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
