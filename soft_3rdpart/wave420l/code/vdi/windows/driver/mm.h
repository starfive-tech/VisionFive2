#ifndef __CNM_VIDEO_MEMORY_MANAGEMENT_H__
#define __CNM_VIDEO_MEMORY_MANAGEMENT_H__

typedef struct _video_mm_info_struct {
	unsigned long   total_pages; 
	unsigned long   alloc_pages; 
	unsigned long   free_pages;
} vmem_info_t;

#if defined (__cplusplus)
extern "C" {
#endif 

	int 
		vmem_init(
		unsigned long addr,
		unsigned long size
		);

	int
		vmem_exit(
		void
		);

	unsigned long
		vmem_alloc(
		int size
		);

	int
		vmem_free(
		unsigned long ptr
		);

	int
		vmem_get_info(
		vmem_info_t* info
		);    
#if defined (__cplusplus)
}
#endif       
#endif /* __CNM_VIDEO_MEMORY_MANAGEMENT_H__ */ 
