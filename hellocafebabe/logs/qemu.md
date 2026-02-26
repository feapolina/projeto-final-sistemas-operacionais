# Felipe Cavalcanti Apolinario  

# SOBRE O QEMU

Basicamente, como o livro é de 2011, algumas coisas mostradas não estão mais compatíveis. O Qemu é o emulador mais moderno que vai dar certo pro problema atual.

Para rodar, precisamos digitar no terminal:

1. make clean

2. make os.iso

3. qemu-system-i386 -cdrom os.iso -m 32 -boot d -no-reboot -no-shutdown

4. qemu-system-i386 -cdrom os.iso -m 32 -boot d -serial stdio

