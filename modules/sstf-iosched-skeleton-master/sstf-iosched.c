/*
 * SSTF IO Scheduler
 *
 * Compatível com kernel 4.13.x (esqueleto fornecido)
 *
 * Lógica:
 * - add_request: insere no final da lista (mantém ordem de chegada)
 * - dispatch: percorre a lista e escolhe o request com menor distância em relação
 *             à posição atual da cabeça (head_pos). Remove esse request e o
 *             despacha com elv_dispatch_sort().
 *
 * Impressões (dmesg):
 * - ao adicionar:  [SSTF] add <pos>
 * - ao despachar:  [SSTF] dsp <pos>
 */

#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/types.h>

/* SSTF data structure. */
struct sstf_data {
	struct list_head queue;
	sector_t head_pos;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	/* Caso do merge, apenas remove 'next' da fila */
	list_del_init(&next->queuelist);
}

/* Esta função despacha o próximo bloco a ser lido. */
/* Dispatch: escolhe o request com menor distância ao head_pos */
static int sstf_dispatch(struct request_queue *q, int force){
	struct sstf_data *nd = q->elevator->elevator_data;
	struct list_head *pos;
	struct request *best = NULL;
	sector_t best_pos = 0;
	unsigned long long best_dist = 0;
	unsigned long flags;
	sector_t cur;
	int found = 0;

	/* protege a fila */
	spin_lock_irqsave(q->queue_lock, flags);

	/* se vazio, nada a fazer */
	if (list_empty(&nd->queue)) {
		spin_unlock_irqrestore(q->queue_lock, flags);
		return 0;
	}

	cur = nd->head_pos;

	/* percorre a lista para encontrar o mais próximo de cur */
	list_for_each(pos, &nd->queue) {
		struct request *rq = list_entry(pos, struct request, queuelist);
		sector_t rq_pos = blk_rq_pos(rq);
		unsigned long long dist;

		if (rq_pos > cur)
			dist = (unsigned long long)(rq_pos - cur);
		else
			dist = (unsigned long long)(cur - rq_pos);

		if (!found || dist < best_dist) {
			best = rq;
			best_pos = rq_pos;
			best_dist = dist;
			found = 1;
		}
	}

	if (best) {
		/* remove da lista e atualiza head_pos */
		list_del_init(&best->queuelist);
		nd->head_pos = best_pos;
	}

	spin_unlock_irqrestore(q->queue_lock, flags);

	if (best) {
		/* despacha */
		elv_dispatch_sort(q, best);
		printk(KERN_INFO "[SSTF] dsp %llu\n", (unsigned long long)best_pos);
		return 1;
	}

	return 0;
}

/* add_request: insere no final da lista (mantém ordem chegada) */
static void sstf_add_request(struct request_queue *q, struct request *rq)
{
	struct sstf_data *nd = q->elevator->elevator_data;
	unsigned long flags;
	sector_t pos;

	/* obter posição para log */
	pos = blk_rq_pos(rq);

	spin_lock_irqsave(q->queue_lock, flags);
	list_add_tail(&rq->queuelist, &nd->queue);
	spin_unlock_irqrestore(q->queue_lock, flags);

	printk(KERN_INFO "[SSTF] add %llu\n", (unsigned long long)pos);
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct sstf_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&nd->queue);
	nd->head_pos = 0; /* inicial: setor 0 (pode ser alterado) */

	eq->elevator_data = nd;

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);

	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
	struct sstf_data *nd = e->elevator_data;

	/* Caso haja requisições restantes (não esperado), apenas loga e limpa */
	if (!list_empty(&nd->queue)) {
		struct request *rq, *tmp;
		list_for_each_entry_safe(rq, tmp, &nd->queue, queuelist) {
			printk(KERN_WARNING "[SSTF] leftover req at pos %llu\n",
			       (unsigned long long)blk_rq_pos(rq));
			list_del_init(&rq->queuelist);
		}
	}

	kfree(nd);
}

/* Infrastrutura dos drivers de IO Scheduling. */
static struct elevator_type elevator_sstf = {
	.ops.sq = {
		.elevator_merge_req_fn		= sstf_merged_requests,
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

/* Inicialização do driver. */
static int __init sstf_init(void)
{
	return elv_register(&elevator_sstf);
}

/* Finalização do driver. */
static void __exit sstf_exit(void)
{
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);

MODULE_AUTHOR("GrupoI");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
