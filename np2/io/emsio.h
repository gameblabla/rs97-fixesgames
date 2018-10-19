
typedef struct {
	UINT8	maxmem;
	UINT8	target;
	UINT16	padding;
	UINT32	addr[4];
} _EMSIO, *EMSIO;


#ifdef __cplusplus
extern "C" {
#endif

void emsio_reset(void);
void emsio_bind(void);

#ifdef __cplusplus
}
#endif

