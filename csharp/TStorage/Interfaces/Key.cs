/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Interfaces
{
    using Cid = int;
    using Mid = long;
    using Moid = int;
    using Cap = long;
    using Acq = long;

    /// <summary>
    /// Represents a unique identifier used in TStorage addressing.
    /// A Key consists of five components:
    /// - Cid: Client ID
    /// - Mid: Meter ID
    /// - Moid: Meter Object ID
    /// - Cap: Capture timestamp
    /// - Acq: Acquisition timestamp
    /// </summary>
    public readonly struct Key : IComparable<Key>, IEquatable<Key>
    {
        /// <summary>
        /// Initializes a new instance of the Key.
        /// </summary>
        /// <param name="cid"> Client ID. </param>
        /// <param name="mid"> Meter ID. </param>
        /// <param name="moid"> Meter Object ID. </param>
        /// <param name="cap"> Capture timestamp. </param>
        /// <param name="acq"> Acquisition timestamp. </param>
        /// <exception cref="ArgumentOutOfRangeException"> When the provided Cid's value is negative. </exception>
        public Key(Cid cid, Mid mid, Moid moid, Cap cap, Acq acq)
        {
            Cid = cid >= CID_MIN_VALUE ? cid : throw new ArgumentOutOfRangeException(nameof(cid), "Cid value cannot be negative.");
            Mid = mid;
            Moid = moid;
            Cap = cap;
            Acq = acq;
        }

        /// <summary> Returns a Key instance with the minimum possible value for each field. </summary>
        public static Key Min()
        {
            return new Key(CID_MIN_VALUE, MID_MIN_VALUE, MOID_MIN_VALUE, CAP_MIN_VALUE, ACQ_MIN_VALUE);
        }

        /// <summary> Returns a Key instance with the maximum possible value for each field. </summary>
        public static Key Max()
        {
            return new Key(CID_MAX_VALUE, MID_MAX_VALUE, MOID_MAX_VALUE, CAP_MAX_VALUE, ACQ_MAX_VALUE);
        }

        /// <summary> Returns the total key size in bytes. </summary>
        public static int StructSize()
        {
            return CID_BYTES + MID_BYTES + MOID_BYTES + CAP_BYTES + ACQ_BYTES;
        }

        public int CompareTo(Key other)
        {
            return (Cid, Mid, Moid, Cap, Acq).CompareTo((other.Cid, other.Mid, other.Moid, other.Cap, other.Acq));
        }

        public bool Equals(Key other)
        {
            return Cid == other.Cid
            && Mid == other.Mid
            && Moid == other.Moid
            && Cap == other.Cap
            && Acq == other.Acq;
        }

        public override bool Equals(object? obj)
        {
            return obj is Key other && Equals(other);
        }

        public override int GetHashCode()
        {
            return HashCode.Combine(Cid, Mid, Moid, Cap, Acq);
        }

        public static bool operator ==(Key left, Key right) => left.Equals(right);

        public static bool operator !=(Key left, Key right) => !left.Equals(right);

        public static bool operator <(Key left, Key right) => left.CompareTo(right) < 0;

        public static bool operator >(Key left, Key right) => left.CompareTo(right) > 0;

        public static bool operator <=(Key left, Key right) => left.CompareTo(right) <= 0;

        public static bool operator >=(Key left, Key right) => left.CompareTo(right) >= 0;

        /// <summary> Minimal valid value for Client Id. </summary>
        public const int CID_MIN_VALUE = 0;

        /// <summary>
        /// Minimal valid value for Meter Id
        /// </summary>
        public const long MID_MIN_VALUE = Mid.MinValue;

        /// <summary>
        /// Minimal valid value for Meter Object Id
        /// </summary>
        public const int MOID_MIN_VALUE = Moid.MinValue;

        /// <summary>
        /// Minimal valid value for Capture time
        /// </summary>
        public const long CAP_MIN_VALUE = Cap.MinValue;

        /// <summary>
        /// Minimal valid value for Acquire time
        /// </summary>
        public const long ACQ_MIN_VALUE = Acq.MinValue;

        /// <summary>
        /// Maximal valid value for Client Id
        /// </summary>
        public const int CID_MAX_VALUE = Cid.MaxValue;

        /// <summary>
        /// Maximal valid value for Meter Id
        /// </summary>
        public const long MID_MAX_VALUE = Mid.MaxValue;

        /// <summary>
        /// Maximal valid value for Meter Object Id
        /// </summary>
        public const int MOID_MAX_VALUE = Moid.MaxValue;

        /// <summary>
        /// Maximal valid value for Capture time
        /// </summary>
        public const long CAP_MAX_VALUE = Cap.MaxValue;

        /// <summary>
        /// Maximal valid value for Acquire time
        /// </summary>
        public const long ACQ_MAX_VALUE = Acq.MaxValue;

        /// <summary> Client Id, must be >= 0 </summary>
        public readonly Cid Cid;
        /// <summary> Meter Id </summary>
        public readonly Mid Mid;
        /// <summary> Meter Object Id </summary>
        public readonly Moid Moid;
        /// <summary> Capture time </summary>
        public readonly Cap Cap;
        /// <summary> Acquisition time </summary>
        public readonly Acq Acq;

        /// <summary> Returns Cid size in bytes. </summary>
        public const int CID_BYTES = sizeof(Cid);

        /// <summary> Returns Mid size in bytes. </summary>
        public const int MID_BYTES = sizeof(Mid);

        /// <summary> Returns Moid size in bytes. </summary>
        public const int MOID_BYTES = sizeof(Moid);

        /// <summary> Returns Cap size in bytes. </summary>
        public const int CAP_BYTES = sizeof(Cap);

        /// <summary> Returns Acq size in bytes. </summary>
        public const int ACQ_BYTES = sizeof(Acq);
    }
}