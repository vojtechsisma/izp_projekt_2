/**
 * @file cluster.c
 * @author Vojtech Sisma (xsisma02@stud.fit.vutbr.cz)
 * @brief IZP projekt 2. - Jednoducha shlukova analyza: 2D nejblizsi soused.
 * @version 0.1
 * @date 2022-12-04
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>   // sqrtf
#include <limits.h> // INT_MAX
#include <string.h>

/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__ \
                            ":%u: "s      \
                            "\n",         \
                            __LINE__, __VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t
{
    int id;
    float x;
    float y;
};

struct cluster_t
{
    int size;
    int capacity;
    struct obj_t *obj;
};

struct config
{
    char *filename;
    int count;
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/**
 * @brief Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 * Ukazatel NULL u pole objektu znamena kapacitu 0.
 *
 * @param c shluk k inicializaci
 * @param cap kapacita shluku
 */
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    c->size = 0;
    c->capacity = cap;

    c->obj = cap > 0 ? malloc(sizeof(struct obj_t) * cap) : NULL;

    if (c->obj == NULL)
    {
        c->capacity = 0;
    }
}

/**
 * @brief Uvolneni pameti a inicializace na prazdny shluk
 *
 * @param c shluk pro odstraneni z pameti
 */
void clear_cluster(struct cluster_t *c)
{
    free(c->obj);
    init_cluster(c, 0);
}

/**
 * @brief uvolneni pameti pole clusteru
 *
 * @param carr pole clusteru
 * @param narr pocet prvku v poli
 */
void clear_clusters(struct cluster_t *carr, int narr)
{
    for (int i = 0; i < narr; i++)
    {
        clear_cluster(&carr[i]);
    }

    free(carr);
}

/* Chunk of cluster objects. Value recommended for reallocation. */
const int CLUSTER_CHUNK = 10;

/**
 * @brief Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 *
 * @param c shluk pro zmenu
 * @param new_cap nova kapacita
 * @return struct cluster_t* pointer na shluk
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t *)arr;
    c->capacity = new_cap;
    return c;
}

/**
 * @brief Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 * nevejde.
 *
 * @param c shluk do ktereho se pridava
 * @param obj objekt pro pridani
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    if (c->size + 1 > c->capacity)
    {
        c = resize_cluster(c, c->capacity + CLUSTER_CHUNK);
    }

    if (c == NULL)
    {
        return;
    }

    c->obj[c->size] = obj;
    c->size++;
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/**
 * @brief  Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 * Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 * Shluk 'c2' bude nezmenen.
 *
 * @param c1 shluk 1
 * @param c2 shluk 2
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    for (int i = 0; i < c2->size; i++)
    {
        append_cluster(c1, c2->obj[i]);
    }

    sort_cluster(c1);
}

/**********************************************************************/
/* Prace s polem shluku */

/**
 * @brief  Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 * (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 * pocet shluku v poli.
 *
 * @param carr pole shluku
 * @param narr pocet prvku v poli shluku
 * @param idx index shluku pro odstraneni
 * @return int pocet prvku v poli po odstraneni
 */
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    clear_cluster(&carr[idx]);
    narr -= 1;
    for (int i = idx; i < narr; i++)
    {
        carr[i] = carr[i + 1];
    }

    return narr;
}

/**
 * @brief Pocita Euklidovskou vzdalenost mezi dvema objekty.
 *
 * @param o1 objekt 1
 * @param o2 objekt 2
 * @return float vzdalenost mezi o1 a o2
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);

    return sqrtf(powf((o1->x - o2->x), 2) + powf((o1->y - o2->y), 2));
}

/**
 * @brief Pocita vzdalenost dvou shluku.
 *
 * @param c1 shluk 1
 * @param c2 shluk 2
 * @return float vzdalenost shluku c1 a c2
 */
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    float distance, min_distance = obj_distance(c1->obj, c2->obj);

    for (int i = 0; i < c1->size; i++)
    {
        for (int j = 0; j < c2->size; j++)
        {
            distance = obj_distance(&c1->obj[i], &c2->obj[j]);

            if (min_distance > distance)
            {
                min_distance = distance;
            }
        }
    }

    return min_distance;
}

/**
 * @brief Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 * hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 * 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 * adresu 'c1' resp. 'c2'.
 *
 * @param carr pole shluku
 * @param narr pocet prvku v poli shluku
 * @param c1 adresa shluku 1
 * @param c2 adresa shluku 2
 */
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);

    float distance, min_distance = cluster_distance(carr, &carr[1]);
    for (int i = 0; i < narr; i++)
    {
        for (int j = i + 1; j < narr; j++)
        {
            distance = cluster_distance(&carr[i], &carr[j]);

            // porovnani vzdalenosti
            if (distance <= min_distance)
            {
                *c1 = i;
                *c2 = j;
                min_distance = distance;
            }
        }
    }
}

/**
 * @brief pomocna funkce pro razeni shluku
 *
 * @param a
 * @param b
 * @return int
 */
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id)
        return -1;
    if (o1->id > o2->id)
        return 1;
    return 0;
}

/**
 * @brief Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
 *
 * @param c shluk pro serazeni
 */
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}

/**
 * @brief Tisk shluku 'c' na stdout.
 *
 * @param c shluk pro vytisk
 */
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i)
            putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/**
 * @brief Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 * jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 * polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 * kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 * V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
 *
 * @param filename nazev souboru se shluky
 * @param arr pole shluku
 * @return int pocet nactenych objektu
 */
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);

    FILE *file;

    file = fopen(filename, "r");

    if (file == NULL)
    {
        return -2;
    }

    char input[101];
    char *count;

    // ziskani poctu objektu
    if (!fgets(input, 100, file))
    {
        return -1;
    }

    strtok(input, "=");
    count = strtok(NULL, "=");
    int c_count;
    int result = sscanf(count, "%d", &c_count);

    if (result == 0 || c_count < 0)
    {
        return -4;
    }

    // alokace pameti pro pole clusteru
    if (!(*arr = malloc(sizeof(struct cluster_t) * c_count)))
    {
        return -1;
    }

    int i = 0;
    while (i < c_count && fgets(input, 100, file))
    {
        init_cluster(&(*arr)[i], 1);

        if (&(*arr)[i] == NULL)
        {
            clear_clusters(*arr, i + 1);
            return -1;
        }

        struct obj_t obj;
        int id, x, y, result;

        // nastaveni vlastnosti objektu ze souboru
        char *token = strtok(input, " ");
        result = sscanf(token, "%d", &id);

        // kontrola vstupu ze souboru
        if (result == 0 || (int)strlen(token) != (int)floor(log10(abs(id))) + 1)
        {
            clear_clusters(*arr, i + 1);
            return -4;
        }

        obj.id = id;

        token = strtok(NULL, " ");
        result = sscanf(token, "%d", &x);

        // kontrola vstupu ze souboru
        if (result == 0 || (int)strlen(token) != (int)floor(log10(abs(x))) + 1 || x < 0 || x > 1000)
        {
            clear_clusters(*arr, i + 1);
            return -4;
        }

        obj.x = x;

        token = strtok(NULL, " ");
        result = sscanf(token, "%d", &y);

        // kontrola vstupu ze souboru
        if (result == 0 || (int)strcspn(token, "\n") != (int)floor(log10(abs(y))) + 1 || y < 0 || y > 1000)
        {
            clear_clusters(*arr, i + 1);
            return -4;
        }

        obj.y = y;

        // pridani clusteru do pole
        append_cluster(&(*arr)[i], obj);

        i++;
    }

    fclose(file);

    return i;
}

/**
 * @brief Tisk pole shluku.
 *
 * @param carr ukazatel na prvni polozku (shluk)
 * @param narr pocet shluku k vytisknuti
 */
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}

/**
 * @brief spojuje clustery dokud neni v poli pozadovany pocet clusteru
 *
 * @param carr pole shluku
 * @param narr pocet clusteru v poli
 * @param size cilova velikost pole
 * @return int nova velikost pole
 */
int compute_required_size(struct cluster_t *carr, int narr, int size)
{
    int c1, c2, prev_c1_size;

    while (narr > size)
    {
        find_neighbours(carr, narr, &c1, &c2);
        prev_c1_size = carr[c1].size;
        merge_clusters(&carr[c1], &carr[c2]);

        if (carr[c1].size != prev_c1_size + carr[c2].size)
        {
            clear_clusters(carr, narr);
            return -1;
        }

        narr = remove_cluster(carr, narr, c2);
    }

    return narr;
}

/**
 * @brief kontrola a nastaveni argumentu
 *
 * @param config struktura pro ulozeni parametru
 * @param argc pocet argumentu
 * @param argv pole argumentu
 */
void process_args(struct config *config, int argc, char *argv[])
{
    config->count = 1;

    if (argc == 2)
    {
        config->filename = argv[1];
    }
    else if (argc == 3)
    {
        config->filename = argv[1];
        int result = sscanf(argv[2], "%d", &config->count);

        // kontrola parametru cisla
        if (result == 0 || (int)strlen(argv[2]) != (int)floor(log10(abs(config->count))) + 1)
        {
            config->count = -3;
        }
    }
    else
    {
        config->count = -3;
    }
}

/**
 * @brief Zobrazuje err hlasky
 *
 * @param error_code kod chyby
 */
void process_error(int error_code)
{
    switch (error_code)
    {
    case -1:
        fputs("Alocation error\n", stderr);
        break;
    case -2:
        fputs("File read error\n", stderr);
        break;
    case -3:
        fputs("Invalid arguments\n", stderr);
        break;
    case -4:
        fputs("Input format error\n", stderr);
        break;
    default:
        fputs("Error\n", stderr);
        break;
    }
}

int main(int argc, char *argv[])
{
    struct config config;

    process_args(&config, argc, argv);

    if (config.count == -3)
    {
        process_error(config.count);
        return 1;
    }

    struct cluster_t *clusters = NULL;

    // nacteni clusteru
    int size = load_clusters(config.filename, &clusters);

    if (clusters == NULL || size < 1)
    {
        process_error(size);
        return 1;
    }

    // uprava na pozadovanou velikost
    size = compute_required_size(clusters, size, config.count);

    if (size < 1)
    {
        process_error(size);
        return 1;
    }

    // vypis clusteru
    print_clusters(clusters, size);

    clear_clusters(clusters, size);

    return 0;
}
