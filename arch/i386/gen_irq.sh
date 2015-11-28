#!/bin/sh

cat <<EOF
#include <config.h>

#ifndef IRQ
#define IRQ(irq)
#endif

#ifndef IRQ_WITH_ERROR
#define IRQ_WITH_ERROR(irq) IRQ(irq)
#endif

EOF

for i in `seq 0 7` 9 `seq 15 47`; do
	echo "IRQ($i)"
done
for i in 8 `seq 10 14`; do
	echo "IRQ_WITH_ERROR($i)"
done

cat <<EOF
#if CONFIG_SWI
IRQ(128)
#endif
#ifndef IRQ_MAX
#if CONFIG_SWI
#define IRQ_MAX 255
#else
#define IRQ_MAX 47
#endif
#endif
EOF
