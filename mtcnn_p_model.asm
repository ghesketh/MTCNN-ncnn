section .rodata

global mtcnn_p_model

mtcnn_p_model:
	align 32
	incbin "mtcnn_p.model"

