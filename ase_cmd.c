#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time.h>

MODULE_LICENSE("Dual BSD/GPL");

#define ASE_BUFFER_LEN 64
static char ase_buffer[ASE_BUFFER_LEN];
static int  ase_flag = 0;
static ssize_t ase_cmd_proc_write(struct file *filp, const char __user *buff,  size_t len, loff_t *data);
static int ase_proc_show(struct seq_file *m, void *v);
static struct proc_dir_entry *root;
static int ase_proc_open(struct inode *inode, struct file *file);


/* usitlise pour savoir si un pid est en route ou pas */
struct pid *pid_struct; //function to find the pid_struct
struct task_struct *task; //find the task_struct

/* used to store the pid number while we open it */
static long current_pid;

/* compteur peremttant de conaitre le nombre de pid cree */
static unsigned long nbr_pid = 0;

#define NBR_PID_SUPPORTED 200

/* gestion des pid */
struct pid_list_s {
  long pid;
  long timestamp;
};
struct pid_list_s * pid_list;

struct timeval current_time;



static const struct file_operations ase_cmd_proc_fops = {
  /* .owner      = THIS_MODULE, */
  .open          = ase_proc_open,
  .read          = seq_read,
  .write         = ase_cmd_proc_write,
  /* .llseek     = seq_lseek, */
  /* .release    = single_release, */
};

/* sends the time back in seconds */
static long get_current_time(void) {
  do_gettimeofday(&current_time);
  return current_time.tv_sec;
}

/* find the timestamp associated with a pid */
static long get_timestamp(unsigned long pid) {
  int i;
  for(i = 0; i < nbr_pid; i++) {
    if(pid_list[i].pid == pid)
      return pid_list[i].timestamp;
  }
  
  /* if we arrive here, we had a problem ! */
  printk(KERN_INFO "We dind't find the requested pid\n");
  return -1;
}


/* ecrit dans le fichier le timestam*/ 
/* TO DO */
/* je n'ai aps compris ce que l'on doit renvoyer a la question 3 */
int
ase_proc_show(struct seq_file *m, void *v)
{
  do_gettimeofday(&current_time);
  seq_printf(m, "%ld\n", get_timestamp(current_pid));
  return 0;
}

/* 
   Gets the name of the proc we want to deal with
   
*/
static int 
ase_proc_open(struct inode *inode, struct file *file)
{

  if(kstrtol(file->f_path.dentry->d_iname, 0, &current_pid) == -ERANGE)  {
    printk(KERN_INFO "We had a problem with tranforming a string into a long\n");
    return  -ERANGE;
  }
  
  return single_open(file, ase_proc_show, NULL);
}


/* Check that the process exist.
   create the process
*/
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
  else if (copy_from_user(ase_buffer, buff, len-1)) {
    return -2;
  }
  /* on init le ase_buffer */
  ase_buffer[len] = 0;
  
  /* tranform une string en long */
  if(kstrtol(ase_buffer, 0, &res) == -ERANGE)  {
    printk(KERN_INFO "We had a problem with tranforming a string into a long\n");
    return  -ERANGE;
  }

  /* on verifie que le param est un PID en route */
  /* merci http://stackoverflow.com/questions/21259672/kernel-right-way-to-check-if-process-is-running-in-c */
  pid_struct = find_get_pid((pid_t)res); //function to find the pid_struct
  task = pid_task(pid_struct,PIDTYPE_PID); //find the task_struct
  /* task renvoit NULL si le PID n'est pas actif */
  if(task == NULL) {
    printk(KERN_INFO "ASE: the PID does not exists\n");
  }
  else {
    /* on verifie que le pid n'a pas deja ete cree */
    /* si c'est le cas, on ne fait rien */
    if(get_timestamp(res)== -1) {
      pid_list[nbr_pid].pid = res;
      pid_list[nbr_pid++].timestamp = task->utime;
      proc_create((const char*)&ase_buffer, 0666, root, &ase_cmd_proc_fops);
      printk(KERN_INFO "the PID has been added\n");
    }  
    else
      printk(KERN_INFO "The pid has already been added\n");
      
  }

  ase_flag = res;

  return len;
}

static int init(void) {
  /* register_jprobe(&my_jprobe); */
  /* on cree un fichier ase_cmd dans /proc/ */
  proc_create("ase_cmd", 0666, NULL, &ase_cmd_proc_fops);
  root = proc_mkdir("ase",NULL);

  pid_list = kmalloc(sizeof(struct pid_list_s) * NBR_PID_SUPPORTED, GFP_KERNEL);
  if (!pid_list) {
    printk(KERN_ALERT "there was a problem with the intialisation of the list of pid\n");
    return  -ERANGE;
  }
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



 
