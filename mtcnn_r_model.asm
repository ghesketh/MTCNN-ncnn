section .rodata

global mtcnn_r_model

mtcnn_r_model:
	align 32
	incbin "mtcnn_r.model"

