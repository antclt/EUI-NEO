# Retained Layer Cache

This document describes the runtime-level retained layer cache. It is a bottom-layer optimization and does not require component API changes.

## Goal

Dirty repaint reduces the repaint area. It does not reduce the number of static primitives that must be replayed inside that area.

The retained layer cache addresses that second cost:

```text
static subtree primitives -> offscreen layer texture
dirty repaint -> draw cached layer texture + dynamic primitives
```

This is useful for complex pages such as Gallery, where a button hover can intersect many static cards, shadows, and text runs.

## Current MVP

The first implementation is intentionally conservative:

- Runtime automatically selects static child subtrees.
- Components do not opt in and do not expose a cache API.
- OpenGL stores each retained layer as a texture-backed framebuffer.
- Non-supporting backends safely fall back to normal primitive replay.
- Backdrop blur and dependent visual subtrees are not cached.
- Animated, interactive, scroll, timer, frame callback, dirty-key, image, and SVG subtrees are not cached.
- A subtree must have enough draw cost and area before it is cached.
- A candidate must be stable for two frames before creating a layer texture.

The cache key includes structure, paint bounds, draw cost, DPI scale, and paint-affecting element properties.

## Hot Path Optimization

After the first MVP shipped, the runtime kept the candidate checks on the render hot path too long. That was fine for complex static pages, but it was wasteful on animation-heavy demos that mostly contain leaf primitives.

The current shape is:

- `layout()` / subtree rebuild caches static blocker flags on `Element`.
- `update()` caches whether a subtree currently has active animation in `PaintBoundsInstance`.
- `render()` only reads those cached flags and skips retained-layer probing for leaf-only children.

This keeps the optimization bottom-layer only, while avoiding repeated subtree recursion during every frame.

## Render Flow

```text
render dirty rect
  traverse ordered children
    if child subtree has a valid retained layer:
      draw layer texture clipped by dirty/scissor
    else if child subtree is a cache candidate:
      render child subtree into a transparent layer texture
      draw layer texture
    else:
      render child subtree normally
```

Layer rebuild disables nested retained-layer use for that subtree. This keeps the first implementation simple and avoids nested framebuffer state surprises.

## Stats

The window title render stats include:

```text
Layer H/M/D/Re
```

- `H`: retained layer cache hits.
- `M`: cache misses or unstable candidates.
- `D`: layer texture draws.
- `Re`: layer texture rebuilds.

Healthy button interaction on a complex static page should trend toward high `H`, low `Re`, and lower primitive draw counts.

## Known Limits

- The MVP caches individual static child subtrees, not merged sibling paint runs yet.
- It currently avoids inherited active transforms for correctness.
- It does not cache backdrop blur because blur samples existing framebuffer content.
- It is not a full retained scene graph or batch renderer.

The next step is to merge adjacent static sibling subtrees into paint runs so several static islands can become one layer draw.
