#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

extern uint get_freemem(void);
extern uint count_active_proc(void);
extern uint count_open_files(void);

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgpte(void)
{
  uint64 va;
  struct proc *p;  

  p = myproc();
  argaddr(0, &va);
  pte_t *pte = pgpte(p->pagetable, va);
  if(pte != 0) {
      return (uint64) *pte;
  }
  return 0;
}
#endif

#ifdef LAB_PGTBL
int
sys_kpgtbl(void)
{
  struct proc *p;  

  p = myproc();
  vmprint(p->pagetable);
  return 0;
}
#endif


uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_hello(void)
{ 
  printf("Hello, world!\n"); 
  return 0; 
} 

uint64
sys_trace(void)
{
    int mask;
    argint(0, &mask);
    
    myproc()->trace_mask = mask;  // Lưu mask vào proc structure
    return 0;
}

uint64
sys_sysinfo(void)
{
  uint64 addr;
  struct sysinfo info;
  argaddr(0, &addr);

  info.freemem = get_freemem();
  info.nproc = count_active_proc();
  info.nopenfiles = count_open_files();

  if (copyout(myproc()->pagetable, addr, (char*)&info, sizeof(info)) < 0)
    return -1;
  
  return 0;
}

uint64
sys_pgaccess(void)
{
  uint64 startva, mask_user;  // Địa chỉ ảo bắt đầu và địa chỉ buffer ở user space
  int npages;

  // Lấy đối số 0: startva, 1: npages, 2: mask_user
  argaddr(0, &startva);
  argint(1, &npages);
  argaddr(2, &mask_user);

  // Đặt giới hạn cho số trang, ví dụ: tối đa 64 trang (bạn có thể điều chỉnh theo yêu cầu)
  if(npages < 1 || npages > 64)
    return -1;
  
  // Biến mask sẽ lưu kết quả bitmask – 1 bit cho mỗi trang
  uint64 mask = 0;

  // Lấy bảng trang của tiến trình hiện tại
  pagetable_t pagetable = myproc()->pagetable;
  
  // Với mỗi trang từ 0 đến npages-1:
  for(int i = 0; i < npages; i++){
    // Tính địa chỉ ảo của trang thứ i (giả sử PGSIZE được định nghĩa là kích thước trang, thường là 4096)
    uint64 va = startva + i * PGSIZE;
    // Dùng hàm walk() để lấy địa chỉ của PTE ứng với địa chỉ ảo. Tham số cuối là 0 nghĩa là không tạo bảng trang mới nếu chưa có.
    pte_t *pte = walk(pagetable, va, 0);
    if(pte != 0 && (*pte & PTE_V)) {
      if(*pte & PTE_A){
        printf("Page %d accessed (va = %p)\n", i, (void*)va);
        mask |= (1ULL << i);  // Đánh dấu trang thứ i đã được truy cập
        *pte &= ~PTE_A;       // Xóa bit Accessed để cho phép theo dõi lần truy cập tiếp theo.
      }
    }
  }
  
  // Copy kết quả mask từ không gian kernel sang không gian người dùng tại địa chỉ mask_user.
  if(copyout(pagetable, mask_user, (char *)&mask, sizeof(mask)) < 0)
    return -1;
  
  return 0;
}