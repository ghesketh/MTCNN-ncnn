section .rodata

global mtcnn_o_model

mtcnn_o_model:
	align 32
	incbin "mtcnn_o.model"
