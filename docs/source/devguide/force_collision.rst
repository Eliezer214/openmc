Forced Collision Biasing
=======================

Overview
--------

This document proposes an initial design for a *forced collision* variance
reduction capability in OpenMC.  Forced collision is a well-known Monte Carlo
biasing technique used to increase the probability of particle interactions
within selected regions, while preserving unbiased estimates through
appropriate weight correction.

The intent of this document is to describe the motivation, high-level algorithm,
and architectural considerations for an initial implementation.  This is a
developer-facing design note and does not constitute a finalized user API.

Motivation
----------

In many transport problems, particles traverse regions of interest with a low
probability of interaction.  Examples include thin materials, small detectors,
or regions of weak coupling where collision-based tallies are statistically
poorly sampled.  In analog Monte Carlo transport, the majority of particle
histories may pass through such regions without contributing to tallies,
leading to high variance.

Forced collision addresses this issue by ensuring that at least one collision
occurs within a selected region, while compensating for the modified sampling
through particle weight adjustment.  This technique is commonly used in
established Monte Carlo transport codes and is complementary to other variance
reduction approaches.

Relation to Existing Biasing in OpenMC
--------------------------------------

OpenMC already supports several biasing mechanisms, including:

- **Survival biasing**, which replaces particle termination upon absorption
  with weight reduction.
- **Weight windows**, which perform particle splitting and roulette based on
  spatially- and energy-dependent target weight ranges.

Forced collision differs from these methods in that it biases *event occurrence*
rather than particle population alone.  Specifically, it modifies the sampling
of collision locations within a region, rather than reacting to particle weight
or reaction outcomes.

Conceptually, forced collision can be viewed as a controlled, event-driven
splitting operation that produces two branches corresponding to collision and
transmission outcomes.

High-Level Algorithm
--------------------

The proposed forced collision implementation operates at the level of a single
cell traversal.  When a particle enters a cell marked for forced collision, the
transport step is modified as follows:

1. Determine the distance :math:`L` to exit the forced-collision cell along the
   particle's current direction.
2. Compute the total macroscopic cross section :math:`\Sigma_t` at the current
   particle energy.
3. Compute the probabilities:

   .. math::

      P_{\text{trans}} = e^{-\Sigma_t L}

      P_{\text{coll}} = 1 - P_{\text{trans}}

4. Create two conceptual branches:

   - **Transmitted branch**:
     - Represents particles that traverse the cell without interaction.
     - Assigned weight :math:`w_{\text{trans}} = w \cdot P_{\text{trans}}`.
     - The particle state is advanced directly to the cell boundary.

   - **Collided branch**:
     - Represents particles that experience at least one collision within the
       cell.
     - Assigned weight :math:`w_{\text{coll}} = w \cdot P_{\text{coll}}`.
     - A collision distance is sampled conditionally such that the collision
       occurs within :math:`[0, L)`.

5. The collided branch continues with standard collision physics, while the
   transmitted branch proceeds with normal transport beyond the cell.

This procedure preserves unbiased estimates by explicitly accounting for both
collision and non-collision outcomes with correct statistical weights.

Scope and Initial Constraints
-----------------------------

The initial implementation is intentionally limited in scope to reduce
complexity and facilitate review:

- Forced collision is applied **at the cell level**.
- Forced collision is applied **at most once per cell traversal**.
- Only neutron transport is considered in the initial design.
- No user-adjustable probability parameters are introduced; the biasing is
  derived solely from physical cross sections and geometry.

Guardrails and Stability Considerations
---------------------------------------

To prevent pathological behavior or particle proliferation, the following
constraints are required:

- Forced collision is applied only once per particle per cell entry.
- Transmitted particles must not immediately re-trigger forced collision due to
  numerical tolerances at cell boundaries.
- Forced collision is skipped when the distance to the cell boundary is
  negligibly small.
- In void regions (:math:`\Sigma_t = 0`), forced collision degenerates to normal
  transport.

These guardrails ensure numerical stability, reproducibility, and compatibility
with existing transport logic.

Tallies and Weight Accounting
-----------------------------

All tallies operate on weighted particle histories as usual.  Track-length and
collision tallies receive contributions from both branches using their
respective weights.  No special tally types are introduced in the initial
implementation.

Care must be taken to ensure that track-length contributions are neither omitted
nor double-counted across forced-collision steps.

Non-Goals
---------

The following items are explicitly out of scope for the initial implementation:

- User-defined collision forcing probabilities.
- Forced collision based on materials, meshes, or energy ranges.
- Adaptive or feedback-driven biasing schemes.
- Coupling with weight window generation or automated variance reduction tools.

These features may be considered in future extensions.

Open Questions and Future Work
------------------------------

Several design questions are intentionally deferred:

- Extension of forced collision beyond cell-based definitions.
- Interaction with advanced variance reduction workflows.
- Integration with hybrid biasing strategies.

These topics can be addressed once a minimal, robust implementation has been
validated.

Summary
-------

This document outlines a minimal, conservative design for introducing forced
collision biasing into OpenMC.  The proposed approach aligns with established
Monte Carlo practices, leverages existing transport infrastructure, and avoids
introducing unnecessary complexity in the initial implementation.
