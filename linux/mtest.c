#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/proc_fs.h>
#include <linux/seq_file.h>/* Needed for seq interface */
#include <linux/sched.h>/* Needed for task_struct*/
#include <linux/mm.h> /* Needed for vm_flags */
#include <asm/uaccess.h>/* Needed for copy_from_user */

#define proc_name  "mtest"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Memory management");
MODULE_AUTHOR("weehowe.z@gmail.com");

char buf[128];/* proc file content */

struct proc_dir_entry *entry;

static void listvma(void){
	struct task_struct *task = current;/* get the task_struct of the current process */
	/* or get the task_struct by pid */
	/* struct task_struct *task = pid_task(find_get_pid(pid),PIDTYPE_PID); */
	struct mm_struct *mm = task->mm;
	struct vm_area_struct *vma = mm->mmap ;/*this struct defines a memory VMM memory area*/

	printk(KERN_INFO "start-addr\tend-addr\tpermission\n");
	while (vma){
		char vma_perm[4];
		//handle premissions
		if (vma->vm_flags & VM_READ) vma_perm[0] = 'r';
			else vma_perm[0] = '-';
		if (vma->vm_flags & VM_WRITE) vma_perm[1] = 'w';
			else vma_perm[1] = '-';
		if (vma->vm_flags & VM_EXEC) vma_perm[2] = 'x';
			else vma_perm[2] = '-';
		vma_perm[3] = '\0';

		printk(KERN_INFO "0x%lx\t0x%lx\t%s\n",vma->vm_start,vma->vm_end,vma_perm);

		vma = vma->vm_next;
	}
}

static int proc_show(struct seq_file *m, void *v) {
	printk(KERN_INFO "/proc/%s read called\n", proc_name);
	seq_printf(m, "%s", buf);
  return 0;
}

static int proc_open(struct inode *inode, struct  file *file) {
  return single_open(file, proc_show, NULL);
}

static ssize_t proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *data){
	printk(KERN_INFO "/proc/%s write called\n", proc_name);
	memset(buf,0,sizeof(buf)/sizeof(char));//clear the buffer

	if (count > sizeof(buf)) return -EINVAL;
	if (copy_from_user(buf, buffer, count)) return -EFAULT;

	if (memcmp(buf,"listvma",7) == 0 && count == 8) {
		printk(KERN_INFO "/proc/%s listvma called\n", proc_name);
		listvma();
	}
	else if (memcmp(buf,"findpage",8) == 0){
		printk(KERN_INFO "/proc/%s findpage called\n", proc_name);

	}
	else if (memcmp(buf,"writeval",8) == 0){
		printk(KERN_INFO "/proc/%s writeval called\n", proc_name);
	}
	return count;
}

static const struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.open = proc_open,
	.read = seq_read,
	.write = proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};

int init_module()
{
	entry = proc_create(proc_name, 0, NULL, &proc_fops);

	if (!entry) {
		remove_proc_entry(proc_name, NULL);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",proc_name);
		return -ENOMEM;
	}
	printk(KERN_INFO "------------\n");
	printk(KERN_INFO "/proc/%s created\n", proc_name);
	return 0;
}

void cleanup_module()
{
	remove_proc_entry(proc_name,NULL);
	printk(KERN_INFO "/proc/%s removed\n", proc_name);
	printk(KERN_INFO "------------\n");
}
