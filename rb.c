#include "DS.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RB_RED 1
#define RB_BLACK 0
/*
  Хоёр оюутны мэдээллийн хооронд нь харьцуулах функц
*/
int less(const Student *a, const Student *b)
{
        if (strcmp(a->id, b->id) < 0)
                return 1;
        return 0;
}

/* ---- Лекцийн LLRB туслах функцүүд ---- */

static int is_red(const Elm *h)
{
        return h != NULL && h->color == RB_RED;
}

static Elm *rotate_left(Elm *h)
{
        Elm *x = h->R;
        h->R = x->L;
        x->L = h;
        x->color = h->color;
        h->color = RB_RED;
        return x;
}

static Elm *rotate_right(Elm *h)
{
        Elm *x = h->L;
        h->L = x->R;
        x->R = h;
        x->color = h->color;
        h->color = RB_RED;
        return x;
}

static void flip_colors(Elm *h)
{
        h->color = 1 - h->color;
        h->L->color = 1 - h->L->color;
        h->R->color = 1 - h->R->color;
}

/* Тэнцвэржүүлэх: рекурсоос буцахдаа дуудна */
static Elm *balance(Elm *h)
{
        if (is_red(h->R) && !is_red(h->L))
                h = rotate_left(h);
        if (is_red(h->L) && is_red(h->L->L))
                h = rotate_right(h);
        if (is_red(h->L) && is_red(h->R))
                flip_colors(h);
        return h;
}

static Elm *new_elm(const Student *px)
{
        Elm *q = malloc(sizeof(Elm));
        if (q == NULL)
                return NULL;
        q->x = *px;
        q->color = RB_RED;
        q->L = NULL;
        q->R = NULL;
        return q;
}

static Elm *put(Elm *h, const Student *px)
{
        if (h == NULL)
                return new_elm(px);
        if (less(px, &h->x))
                h->L = put(h->L, px);
        else if (less(&h->x, px))
                h->R = put(h->R, px);
        else
                h->x = *px;             /* id адилхан бол мэдээллийг шинэчилнэ */

        if (is_red(h->R) && !is_red(h->L))
                h = rotate_left(h);
        if (is_red(h->L) && is_red(h->L->L))
                h = rotate_right(h);
        if (is_red(h->L) && is_red(h->R))
                flip_colors(h);
        return h;
}

static Elm *move_red_left(Elm *h)
{
        flip_colors(h);
        if (is_red(h->R->L)) {
                h->R = rotate_right(h->R);
                h = rotate_left(h);
                flip_colors(h);
        }
        return h;
}

static Elm *move_red_right(Elm *h)
{
        flip_colors(h);
        if (is_red(h->L->L)) {
                h = rotate_right(h);
                flip_colors(h);
        }
        return h;
}

static Elm *min_elm(Elm *h)
{
        while (h->L != NULL)
                h = h->L;
        return h;
}

static Elm *delete_min(Elm *h)
{
        if (h->L == NULL) {
                free(h);
                return NULL;
        }
        if (!is_red(h->L) && !is_red(h->L->L))
                h = move_red_left(h);
        h->L = delete_min(h->L);
        return balance(h);
}

/* id модонд заавал байгаа үед дуудна */
static Elm *del(Elm *h, const char id[])
{
        if (strcmp(id, h->x.id) < 0) {
                if (!is_red(h->L) && !is_red(h->L->L))
                        h = move_red_left(h);
                h->L = del(h->L, id);
        } else {
                if (is_red(h->L))
                        h = rotate_right(h);
                if (strcmp(id, h->x.id) == 0 && h->R == NULL) {
                        free(h);
                        return NULL;
                }
                if (!is_red(h->R) && !is_red(h->R->L))
                        h = move_red_right(h);
                if (strcmp(id, h->x.id) == 0) {
                        /* successor-ийн мэдээллийг хуулж, successor-ийг устгана */
                        h->x = min_elm(h->R)->x;
                        h->R = delete_min(h->R);
                } else
                        h->R = del(h->R, id);
        }
        return balance(h);
}

/*
  `ptree`-ийн зааж байгаа модонд `x` утгыг оруулна.
  Мод хоосон байсан бол `ptree->root` хаяг өөрчлөгдөж шинээр орсон оройг заана.
  Хэрэв мод тэнцвэрээ алдсан бол тохирох тэнцвэржүүлэх үйлдлүүдийг хийнэ.
 */
void rb_put(RBT *ptree, const Student *px)
{
        ptree->root = put(ptree->root, px);
        ptree->root->color = RB_BLACK;
}

/*
  `ptree`-ийн зааж байгаа модноос `x` утгыг хайн олдсон оройн `Elm*` хаягийг буцаана.
  Олдохгүй бол `NULL` хаягийг буцаана.
 */
Elm *rb_get(const RBT *ptree, const char id[])
{
        Elm *t = ptree->root;
        while (t != NULL) {
                int c = strcmp(id, t->x.id);
                if (c < 0)
                        t = t->L;
                else if (c > 0)
                        t = t->R;
                else
                        return t;
        }
        return NULL;
}

/*
  Устгах функц: ТМноос `x` утгыг хайж олоод устгана.
  Олдохгүй бол юу ч хийхгүй.
  Хэрэв мод тэнцвэрээ алдсан бол тохирох тэнцвэржүүлэх үйлдлүүдийг хийнэ.
 */
void rb_del(RBT *ptree, const char id[])
{
        if (rb_get(ptree, id) == NULL)
                return;
        if (!is_red(ptree->root->L) && !is_red(ptree->root->R))
                ptree->root->color = RB_RED;
        ptree->root = del(ptree->root, id);
        if (ptree->root != NULL)
                ptree->root->color = RB_BLACK;
}

void print(Elm *p)
{
        if (p)
                printf("%s %s %d %.1f\n", p->x.name, p->x.id, p->x.age, p->x.gpa);
        else
                printf("None\n");
}
