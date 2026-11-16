#include "DS.h"
#include <stdio.h>

/*
  Хоёр оюутны мэдээллийн хооронд нь харьцуулах функц
  a-ийн id нь b-ийнхээс цагаан толгойн дарааллаар бага бол 1, үгүй бол 0
*/
int less(const Student *a, const Student *b)
{
        if (strcmp(a->id, b->id) < 0)
                return 1;
        return 0;
}

/* ---- Лекцийн слайдын элементар үйлдлүүд ---- */

/* null холбоос хар */
static int isRed(const Elm *x)
{
        if (x == NULL)
                return 0;
        return x->color == RB_RED;
}

/* Баруун налуу улаан холбоосыг зүүн налуу болгоно */
static Elm *rotateLeft(Elm *h)
{
        Elm *x = h->R;
        h->R = x->L;
        x->L = h;
        x->color = h->color;
        h->color = RB_RED;
        return x;
}

/* Зүүн налуу улаан холбоосыг түр баруун налуу болгоно */
static Elm *rotateRight(Elm *h)
{
        Elm *x = h->L;
        h->L = x->R;
        x->R = h;
        x->color = h->color;
        h->color = RB_RED;
        return x;
}

/* Өнгө эргүүлэх. Оруулахад слайдтай адил (h улаан, хүүхдүүд хар) болно.
   Устгахад хоёр чиглэлд хэрэгтэй тул эргүүлэх хэлбэрээр бичсэн. */
static void flipColors(Elm *h)
{
        h->color = !h->color;
        h->L->color = !h->L->color;
        h->R->color = !h->R->color;
}

static Elm *newElm(const Student *px)
{
        Elm *q = malloc(sizeof(Elm));
        if (q == NULL)
                exit(1);
        q->x = *px;
        q->color = RB_RED;
        q->L = NULL;
        q->R = NULL;
        return q;
}

/* ---- Оруулах (слайд 37-ын put) ---- */
static Elm *put(Elm *h, const Student *px)
{
        if (h == NULL)
                return newElm(px);

        if (less(px, &h->x))
                h->L = put(h->L, px);
        else if (less(&h->x, px))
                h->R = put(h->R, px);
        else
                h->x = *px;             /* id адил бол мэдээллийг шинэчилнэ */

        if (isRed(h->R) && !isRed(h->L))
                h = rotateLeft(h);
        if (isRed(h->L) && isRed(h->L->L))
                h = rotateRight(h);
        if (isRed(h->L) && isRed(h->R))
                flipColors(h);
        return h;
}

/* ---- Устгахад хэрэгтэй туслахууд (слайдад байхгүй, Sedgewick-ийн номоос) ---- */

static Elm *fixUp(Elm *h)
{
        if (isRed(h->R) && !isRed(h->L))
                h = rotateLeft(h);
        if (isRed(h->L) && isRed(h->L->L))
                h = rotateRight(h);
        if (isRed(h->L) && isRed(h->R))
                flipColors(h);
        return h;
}

static Elm *moveRedLeft(Elm *h)
{
        flipColors(h);
        if (isRed(h->R->L)) {
                h->R = rotateRight(h->R);
                h = rotateLeft(h);
                flipColors(h);
        }
        return h;
}

static Elm *moveRedRight(Elm *h)
{
        flipColors(h);
        if (isRed(h->L->L)) {
                h = rotateRight(h);
                flipColors(h);
        }
        return h;
}

static Elm *minElm(Elm *h)
{
        while (h->L != NULL)
                h = h->L;
        return h;
}

static Elm *deleteMin(Elm *h)
{
        if (h->L == NULL) {
                free(h);
                return NULL;
        }
        if (!isRed(h->L) && !isRed(h->L->L))
                h = moveRedLeft(h);
        h->L = deleteMin(h->L);
        return fixUp(h);
}

/* id модонд заавал байгаа үед дуудна */
static Elm *del(Elm *h, const char id[])
{
        if (strcmp(id, h->x.id) < 0) {
                if (!isRed(h->L) && !isRed(h->L->L))
                        h = moveRedLeft(h);
                h->L = del(h->L, id);
        } else {
                if (isRed(h->L))
                        h = rotateRight(h);
                if (strcmp(id, h->x.id) == 0 && h->R == NULL) {
                        free(h);
                        return NULL;
                }
                if (!isRed(h->R) && !isRed(h->R->L))
                        h = moveRedRight(h);
                if (strcmp(id, h->x.id) == 0) {
                        h->x = minElm(h->R)->x;         /* successor-оор солино */
                        h->R = deleteMin(h->R);
                } else
                        h->R = del(h->R, id);
        }
        return fixUp(h);
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
  Мод дандаа ялгаатай элементүүд хадгална гэж үзэж болно.
 */
Elm *rb_get(const RBT *ptree, const char id[])
{
        Elm *x = ptree->root;
        while (x != NULL) {
                int cmp = strcmp(id, x->x.id);
                if (cmp < 0)
                        x = x->L;
                else if (cmp > 0)
                        x = x->R;
                else
                        return x;
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
        if (!isRed(ptree->root->L) && !isRed(ptree->root->R))
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
