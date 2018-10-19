
typedef struct {
	int		enable;
} _NMIIO, *NMIIO;


#ifdef __cplusplus
extern "C" {
#endif

void nmiio_reset(void);
void nmiio_bind(void);

#ifdef __cplusplus
}
#endif

