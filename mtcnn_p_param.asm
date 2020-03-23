section .rodata

global mtcnn_p_param

mtcnn_p_param:
	align 32
	incbin "mtcnn_p.param"

