// Copyright (c) ZeroC, Inc.

package com.zeroc.ice.slice.tools

import org.w3c.dom.Node
import org.w3c.dom.NodeList

/**
 * Converts a NodeList into a Kotlin Sequence.
 *
 * This allows easy iteration over XML nodes using Kotlin's sequence functions.
 */
fun NodeList.toSequence(): Sequence<Node> =
    (0 until length).asSequence().map { item(it) }
