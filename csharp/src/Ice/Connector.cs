// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

namespace ZeroC.Ice
{
    /// <summary>A connector holds all the information needed to establish a connection to a remote peer. It creates
    /// a transceiver that can be used to exchange data with the remote peer once the connection is established.
    /// </summary>
    public abstract class Connector : IEquatable<Connector>
    {
        /// <summary>Gets the <see cref="Endpoint"/> of this connector.</summary>
        public abstract Endpoint Endpoint { get; }

        /// <summary>The equality operator == returns true if its operands are equal, false otherwise.</summary>
        /// <param name="lhs">The left hand side operand.</param>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if the operands are equal, otherwise <c>false</c>.</returns>
        public static bool operator ==(Connector? lhs, Connector? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs is null || rhs is null)
            {
                return false;
            }
            return rhs.Equals(lhs);
        }

        /// <summary>The inequality operator != returns true if its operands are not equal, false otherwise.</summary>
        /// <param name="lhs">The left hand side operand.</param>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if the operands are not equal, otherwise <c>false</c>.</returns>
        public static bool operator !=(Connector? lhs, Connector? rhs) => !(lhs == rhs);

        /// <summary>Creates a transceiver. The transceiver may not be fully connected until its
        /// <see cref="ITransceiver.InitializeAsync"/> method is called.</summary>
        /// <return>The transceiver.</return>
        public abstract Connection Connect(string connectionId);

        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is Connector other && Equals(other);

        /// <inheritdoc/>
        public abstract bool Equals(Connector? obj);

        /// <inheritdoc/>
        public abstract override int GetHashCode();
    }
}
