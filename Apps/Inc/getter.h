#define GETTER(type, name) \
    type get_##name(void){ \
        return name; \
    } \

#define GETTER_H(type, name) \
    type get_##name(void); \

