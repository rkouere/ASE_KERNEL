#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
MODULE_LICENSE("Dual BSD/GPL");

#define ASE_BUFFER_LEN 64
static char ase_buffer[ASE_BUFFER_LEN];
static int  ase_flag = 0;
static ssize_t ase_cmd_proc_write(struct file *filp, const char __user *buff,  size_t len, loff_t *data);
static struct proc_dir_entry *root;

/* usitlise pour savoir si un pid est en route ou pas */
struct pid *pid_struct; //function to find the pid_struct
struct task_struct *task; //find the task_struct


/* int my_callback(struct task_struct *p, int policy, const struct sched_param *param) { */
/*   printk(KERN_INFO "!!!!!!! we are the champions !!!!!!!!!!!!\n");   */
/*   printk(KERN_INFO "!!!!!!! we are the champions !!!!!!!!!!!!\n"); */
/* } */

/* static struct jprobe my_jprobe = { */
/*   .kp = { */
/*     .symbol_name = "sched_setscheduler", */
/*   }, */
/*   .entry = (kprobe_opcode_t *) my_callback */
/* }; */

static const struct file_operations ase_cmd_proc_fops = {
  /* .owner      = THIS_MODULE, */
  /* .open       = jiffies_proc_open, */
  /* .read       = seq_read, */
  .write         = ase_cmd_proc_write,
  /* .llseek     = seq_lseek, */
  /* .release    = single_release, */
};



static ssize_t
ase_cmd_proc_write(struct file *filp, const char __user *buff,
		   size_t len, loff_t *data)
{
  long res;
  printk(KERN_INFO "ASE: Write has been called");
  /* on verifie que la taille du buffer passe en param n'est pas plus grande qu'une plage memoire */
  if (len > (ASE_BUFFER_LEN - 1)) {
    printk(KERN_INFO "ASE: error, input too long");
    return -EINVAL;
  }
  /* sinon on copie le contenu du buffer dans notre ase_buffer (buff est dans l'espace utilisateur et on ne peut rien faire avec) */
  else if (copy_from_user(ase_buffer, buff, len)) {
    return -2;
  }
  /* on init le ase_buffer */
  ase_buffer[len] = 0;
  
  /* tranform une string en long */
  kstrtol(ase_buffer, 0, &res);

  /* on verifie que le param est un PID en route */
  /* merci http://stackoverflow.com/questions/21259672/kernel-right-way-to-check-if-process-is-running-in-c */
  pid_struct = find_get_pid((pid_t)res); //function to find the pid_struct
  task = pid_task(pid_struct,PIDTYPE_PID); //find the task_struct
  /* task renvoit NULL si le PID n'est pas actif */
  if(task == NULL)
    printk(KERN_INFO "ASE: task not exists");
  else {
    proc_create(&ase_buffer, 0666, root, &ase_cmd_proc_fops);
    printk(KERN_INFO "ASE: task exists");
  }


  /* cree un process avec le param */
  
  ase_flag = res;





  return len;
}

static int init(void) {
  /* register_jprobe(&my_jprobe); */
  /* on cree un fichier ase_cmd dans /proc/ */
  proc_create("ase_cmd", 0666, NULL, &ase_cmd_proc_fops);
  root = proc_mkdir("ase",NULL);
  printk(KERN_ALERT "INIT ase_cmd\n");
  return 0;
}
static void hello_exit(void)
{
  /* unregister_jprobe(&my_jprobe); */
  remove_proc_entry("ase_cmd", NULL);
  remove_proc_entry("ase", NULL);
  printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(init);
module_exit(hello_exit);



 
