# Multiobjective Combinatorial Optimization Instances

This directory contains instances for some of the implemented Multiobjective Combinatorial Optimization Problems, specifically the Multiobjective Knapsack Problem (MOKP) and the Multiobjective Minimum Spanning Tree (MOMST).

## Instance Types

The repository includes randomly generated instances for both problem types:

- MOKP: Random instances generated using the [mobkp-instances](https://github.com/gaplopes/mobkp-instances) repository.
- MOMST: Complete graph instances generated using similar parameters to the random MOKP instances.

In the original research paper, the following benchmark instances from literature were also used for experimental evaluation:

- MOKP: Instances from [Bazgan et al. (2009)](https://doi.org/10.1016/j.cor.2007.09.009)

- MOMST: Instances from [Fernandes et al. (2020)](https://doi.org/10.1007/s10589-019-00154-1)

## Multiobjective Knapsack Problem (MOKP)

The MOKP instance files follow this format:

```
n m                   // number of items (n), number of objectives (m)
W                     // capacity
w1 v11 v12 ... v1m    // values and weight for item 1
w2 v21 v22 ... v2m    // values and weight for item 2
...
wn vn1 vn2 ... vnm    // values and weight for item n
```

If the instance file as the non-dominated set of solutions, it should be placed at the end of the file:

```
n m                   // number of items (n), number of objectives (m)
W                     // capacity
w1 v11 v12 ... v1m    // values and weight for item 1
w2 v21 v22 ... v2m    // values and weight for item 2
...
wn vn1 vn2 ... vnm    // values and weight for item n
n_nondominated_set    // number of solutions in the non-dominated set
v11 v12 ... v1m       // values for solution 1
v21 v22 ... v2m       // values for solution 2
...
vn v1 v2 ... vm       // values for solution n
```

### Example
```
3 2       // 3 items, 2 objectives
10        // capacity 10
4 5 3     // item 1: values [5,3], weight 4
3 2 4     // item 2: values [2,4], weight 3
5 3 5     // item 3: values [3,5], weight 5
```

The MOKP instances are randomly generated instances from the [mobkp-instances](https://github.com/gaplopes/mobkp-instances).

## Multiobjective Minimum Spanning Tree (MOMST)

The MOMST instance files follow this format:

```
V E M      // number of vertices (V), edges (E), objectives (M)
s1 d1 v11 v12 ... v1M    // edge 1: source, destination, values
s2 d2 v21 v22 ... v2M    // edge 2: source, destination, values
...
sE dE vE1 vE2 ... vEM    // edge E: source, destination, values
```

If the instance file as the non-dominated set of solutions, it should be placed at the end of the file:

```
V E M      // number of vertices (V), edges (E), objectives (M)
s1 d1 v11 v12 ... v1M    // edge 1: source, destination, values
s2 d2 v21 v22 ... v2M    // edge 2: source, destination, values
...
sE dE vE1 vE2 ... vEM    // edge E: source, destination, values
n_nondominated_set      // number of solutions in the non-dominated set
v11 v12 ... v1M         // values for solution 1
v21 v22 ... v2M         // values for solution 2
...
vN v1 v2 ... vM         // values for solution N
```

### Example
```
4 5 2      // 4 vertices, 5 edges, 2 objectives
0 1 2 3    // edge 1: 0->1, values [2,3]
0 2 3 2    // edge 2: 0->2, values [3,2]
1 2 1 4    // edge 3: 1->2, values [1,4]
1 3 4 1    // edge 4: 1->3, values [4,1]
2 3 2 2    // edge 5: 2->3, values [2,2]
```

The MOMST instances are complete graphs generated based on the same source script as the random MOKP instances. (see [mobkp-instances](https://github.com/gaplopes/mobkp-instances)).
